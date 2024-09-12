#include "pch.h"

#include "player_pickup_component.h"
#include <core/profiler.h>
#include <core/engine_wrapper.h>

#include <game/components/pickupable_component.h>
#include <game/components/controller_component.h>
#include <game/components/physics_component.h>

static std::vector<Entity*> get_entities_from_collision_result(Scene* scn, std::vector<JPH::Body*> bodies)
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



void PlayerPickupComponent::update()
{
	QUE_PROFILE;

	auto grab_state = g_engine.input->get_grab_state()[m_controller->index];

	if (grab_state > 0.5f && this->m_pickuped_object == nullptr)
	{
		try_pickup();

	}
	else if (grab_state < 0.5f && this->m_pickuped_object != nullptr)
	{
		try_release();
	}
}

void PlayerPickupComponent::init()
{
	m_controller = entity->get<ControllerComponent>();
}

void PlayerPickupComponent::try_release()
{
	//mic.busy = false;
	//reg.remove<attach_component>(ent);

	//transform_component* picked_object_tc = reg.try_get<transform_component>(ent);

	//physics_component* pc = reg.try_get<physics_component>(ent);
	//if (pc)
	//{
	//	psys.set_body_rotation(pc->id, picked_object_tc->rotation);
	//	psys.add_velocity(pc->id, cc.vel * 10);
	//	psys.set_motion_type(pc->id, JPH::EMotionType::Dynamic);
	//	pc->enabled = false;
	//}
}

void PlayerPickupComponent::try_pickup()
{
	auto bodies = g_engine.physics->overlap_sphere(entity->position, 0.0001f);
	auto entities = get_entities_from_collision_result(this->entity->scene, bodies);

	if (entities.size() == 0)
		return;

	// find an entity that has pickupable component
	Entity* ent = nullptr;
	for (auto& e : entities)
	{
		auto pb = e->get<PickupableComponent>();
		if (pb)
		{
			ent = e;
			break;
		}
	}

	if (ent == nullptr)
		return;

	m_pickuped_object = ent;

	/*glm::quat offset = glm::inverse(tc.rotation) * picked_object_tc->rotation;
	picked_object_tc->rotation2 = offset;*/
	
	ent->set_parent(this->entity);

	PhysicsComponent* pc = ent->get<PhysicsComponent>();
	if (pc)
	{
		g_engine.physics->set_motion_type(pc->get_body_id(), JPH::EMotionType::Kinematic);
		pc->kinematic = true;
	}
}
