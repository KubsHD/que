#include "pch.h"

#include "player_pickup_component.h"
#include <core/profiler.h>
#include <core/engine_wrapper.h>

#include <game/components/pickupable_component.h>
#include <game/components/controller_component.h>
#include <game/components/physics_component.h>
#include "collision_helpers.h"





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

	if (m_pickuped_object)
	{
		m_pickuped_object->position = entity->position;
		m_pickuped_object->rotation = entity->rotation * m_pickuped_object_original_rotation;
	}
}

void PlayerPickupComponent::init()
{
	m_controller = entity->get<ControllerComponent>();
}

void PlayerPickupComponent::try_release()
{
	m_pickuped_object->set_parent(nullptr);

	PhysicsComponent* pc = m_pickuped_object->get<PhysicsComponent>();
	if (pc)
	{
		g_engine.physics->set_body_rotation(pc->get_body_id(), m_pickuped_object->rotation);
		g_engine.physics->set_motion_type(pc->get_body_id(), JPH::EMotionType::Dynamic);
		g_engine.physics->add_velocity(pc->get_body_id(), m_controller->get_velocity() * 100);
		pc->kinematic = false;
	}

	m_pickuped_object = nullptr;
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

	m_pickuped_object_original_rotation = glm::inverse(entity->rotation) * m_pickuped_object->rotation;

	ent->set_parent(this->entity);

	PhysicsComponent* pc = ent->get<PhysicsComponent>();
	if (pc)
	{
		g_engine.physics->set_motion_type(pc->get_body_id(), JPH::EMotionType::Kinematic);
		pc->kinematic = true;
	}
}
