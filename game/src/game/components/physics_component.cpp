#include "pch.h"

#include "physics_component.h"

PhysicsComponent::PhysicsComponent(JPH::BodyCreationSettings bodySettings)
{
	m_body = g_engine.physics->spawn_body(bodySettings);
}

void PhysicsComponent::init()
{
}

void PhysicsComponent::update()
{
	if (!kinematic)
	{
		this->entity->position = this->entity->scene->engine.physics->get_body_position(m_body);
		this->entity->rotation = this->entity->scene->engine.physics->get_body_rotation(m_body);
	}
	else 
	{
		g_engine.physics->set_body_position(m_body, this->entity->position);
		g_engine.physics->set_body_rotation(m_body, this->entity->rotation);
	}
}

JPH::BodyID PhysicsComponent::get_body_id() const
{
	return m_body;
}
