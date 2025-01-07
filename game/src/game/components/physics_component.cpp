#include "pch.h"

#include "physics_component.h"
#include <core/physics_util.h>

#include <Jolt\Physics\Body\BodyCreationSettings.h>

PhysicsComponent::PhysicsComponent(JPH::BodyCreationSettings bodySettings)
{
	m_body = g_engine.physics->spawn_body(bodySettings);
	m_collider_type = ColliderType::Dynamic;
}

PhysicsComponent::PhysicsComponent(Mesh& mesh)
{
	m_collider_type = ColliderType::Static;
	m_mesh = mesh;
}

void PhysicsComponent::init()
{
	if (m_collider_type == ColliderType::Static)
	{
		JPH::BodyCreationSettings obj_settings(
			core::physics::create_mesh_shape(m_mesh, entity->scale),
			JPH::to_jph(entity->position),
			JPH::to_jph(entity->rotation),
			JPH::EMotionType::Static,
			Layers::NON_MOVING);

		obj_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
		obj_settings.mMassPropertiesOverride.mMass = 100.0f;
		obj_settings.mMassPropertiesOverride.mInertia = JPH::Mat44::sIdentity();

		m_body = g_engine.physics->spawn_body(obj_settings);

		kinematic = true;
	}
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
