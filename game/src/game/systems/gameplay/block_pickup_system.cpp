#include "pch.h"

#include "block_pickup_system.h"
#include <game/tags.h>
#include <core/profiler.h>
#include <common/DebugOutput.h>


static std::vector<entt::entity> get_entities_from_collision_result(entt::registry& reg, std::vector<JPH::Body*> bodies)
{
	std::vector<entt::entity> entities;

	// we need to pair bodyID with a entt::entity
	std::unordered_map<int, entt::entity> body_to_entity_map;

	auto view = reg.view<physics_component>();

	auto a = view.each();

	auto end = std::cend(view);

	for (auto body : bodies)
	{
		auto ent = std::find_if(std::cbegin(view), end, [&](const auto& entity) {
			const physics_component& myTag = view.get<physics_component>(entity);
			for (auto body : bodies)
				if (myTag.id == body->GetID())
					return true;

			return false;
			});

		if (ent != end)
			entities.push_back(*ent);
	}

	return entities;
}


void game::system::update_block_pickup_system(entt::registry& reg, Input& inp, PhysicsSystem& psys)
{
	QUE_PROFILE;

	// for each controller check if we overlap an interactable block
	// if yes, then attach the block to controller position
	// if a block was already attached and player unpressed hold button, detach the block from controller and run its physics simulation

	for (const auto& [e, tc, cc] : reg.view<transform_component, controller_component>().each())
	{

		auto bodies = psys.overlap_sphere(tc.position, 0.0001f);
		auto entities = get_entities_from_collision_result(reg, bodies);


		// we got all entities that overlap the sphere
		// now we have to find the one with a component pickupable_block
		auto view = reg.view<pickupable_block>();

		for (auto& ent : entities)
		{
			const pickupable_block* pb = reg.try_get<pickupable_block>(ent);

			if (pb)
			{
				auto grab_state = inp.get_grab_state()[cc.index];

				if (grab_state > 0.5f)
				{
					if (reg.try_get<attach_component>(ent) == nullptr)
					{
						XR_TUT_LOG("attaching to controller index: " << cc.index << std::endl);

						reg.emplace_or_replace<attach_component>(ent, e);

						// disable physics if present
						physics_component* pc = reg.try_get<physics_component>(ent);
						if (pc)
							pc->enabled = false;
					}
				}
				else {
					reg.remove<attach_component>(ent);

					// enable physics if present
					physics_component* pc = reg.try_get<physics_component>(ent);
					if (pc)
						pc->enabled = true;
				}

				break;
			}
		}
	}
}
