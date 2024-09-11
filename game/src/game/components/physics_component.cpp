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
	this->entity->position = this->entity->scene->engine.physics->get_body_position(m_body);
	this->entity->rotation = this->entity->scene->engine.physics->get_body_rotation(m_body);
}
