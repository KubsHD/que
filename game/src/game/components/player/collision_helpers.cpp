#include "pch.h"

#include "collision_helpers.h"

#include <game/components/physics_component.h>


std::vector<Entity*> get_entities_from_collision_result(Scene* scn, std::vector<JPH::Body*> bodies)
{
	std::vector<Entity*> entities;

	// we need to pair bodyID with a entt::entity
	std::unordered_map<int, Entity*> body_to_entity_map;

	Vector<PhysicsComponent*> view = scn->get_all_components_of_type<PhysicsComponent>();
	auto end = std::cend(view);

	for (auto body : bodies)
	{
		auto ent = std::find_if(std::cbegin(view), end, [&](auto entity_pc) {
			for (auto body : bodies)
				if (entity_pc->get_body_id() == body->GetID())
					return true;

			return false;
			});

		if (ent != end)
			entities.push_back((*ent)->entity);

	}

	return entities;
}
