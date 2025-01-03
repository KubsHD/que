#include "pch.h"

#include "physics_component.h"
#include <core/physics_util.h>

PhysicsComponent::PhysicsComponent(JPH::BodyCreationSettings bodySettings)
{
	m_body = g_engine.physics->spawn_body(bodySettings);
}

PhysicsComponent::PhysicsComponent(Mesh& mesh, JPH::BodyCreationSettings bodySettings /*= {}*/)
{
	JPH::BodyCreationSettings obj_settings(
		core::physics::create_mesh_shape(mesh),
		JPH::RVec3(0, -1.0f, 0),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Static,
		Layers::NON_MOVING);

	obj_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
	obj_settings.mMassPropertiesOverride.mMass = 100.0f;
	obj_settings.mMassPropertiesOverride.mInertia = JPH::Mat44::sIdentity();

	m_body = g_engine.physics->spawn_body(obj_settings);

	kinematic = true;
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
