#include "pch.h"

#include "block_pickup_system.h"
#include <core/profiler.h>
#include <common/DebugOutput.h>
#include <core/physics.h>
#include <core/xr/xr_input.h>
//
//
//static std::vector<entt::entity> get_entities_from_collision_result(entt::registry& reg, std::vector<JPH::Body*> bodies)
//{
//	std::vector<entt::entity> entities;
//
//	// we need to pair bodyID with a entt::entity
//	std::unordered_map<int, entt::entity> body_to_entity_map;
//
//	auto view = reg.view<physics_component>();
//
//	auto a = view.each();
//
//	auto end = std::cend(view);
//
//	for (auto body : bodies)
//	{
//		auto ent = std::find_if(std::cbegin(view), end, [&](const auto& entity) {
//			const physics_component& myTag = view.get<physics_component>(entity);
//			for (auto body : bodies)
//				if (myTag.id == body->GetID())
//					return true;
//
//			return false;
//			});
//
//		if (ent != end)
//			entities.push_back(*ent);
//	}
//
//	return entities;
//}
//
//
//void game::system::update_block_pickup_system(entt::registry& reg, Input& inp, PhysicsSystem& psys)
//{
//	QUE_PROFILE;
//
//	// for each controller check if we overlap an interactable block
//	// if yes, then attach the block to controller position
//	// if a block was already attached and player unpressed hold button, detach the block from controller and run its physics simulation
//
//	for (const auto& [e, tc, cc, mic] : reg.view<transform_component, controller_component, main_interacting_controller>().each())
//	{
//
//		auto bodies = psys.overlap_sphere(tc.position, 0.0001f);
//		auto entities = get_entities_from_collision_result(reg, bodies);
//
//
//		// we got all entities that overlap the sphere
//		// now we have to find the one with a component pickupable_block
//		auto view = reg.view<pickupable_block>();
//
//		for (auto& ent : entities)
//		{
//			const pickupable_block* pb = reg.try_get<pickupable_block>(ent);
//
//			if (pb)
//			{
//				auto grab_state = inp.get_grab_state()[cc.index];
//				auto has_attach_component = reg.try_get<attach_component>(ent) != nullptr;
//
//				if (grab_state > 0.5f && !has_attach_component && !mic.busy)
//				{
//					mic.busy = true;
//					reg.emplace<attach_component>(ent, e);
//
//					transform_component* picked_object_tc = reg.try_get<transform_component>(ent);
//
//					glm::quat offset = glm::inverse(tc.rotation) * picked_object_tc->rotation;
//					picked_object_tc->rotation2 = offset;
//
//					physics_component* pc = reg.try_get<physics_component>(ent);
//					if (pc)
//					{
//						psys.set_motion_type(pc->id, JPH::EMotionType::Kinematic);
//						pc->enabled = false;
//					}
//				}
//				else if (grab_state < 0.5f && has_attach_component)
//				{
//					mic.busy = false;
//					reg.remove<attach_component>(ent);
//
//					transform_component* picked_object_tc = reg.try_get<transform_component>(ent);
//
//					physics_component* pc = reg.try_get<physics_component>(ent);
//					if (pc)
//					{
//						psys.set_body_rotation(pc->id, picked_object_tc->rotation);
//						psys.add_velocity(pc->id, cc.vel * 10);
//						psys.set_motion_type(pc->id, JPH::EMotionType::Dynamic);
//						pc->enabled = false;
//					}
//				}
//			}
//		}
//	}
//}
