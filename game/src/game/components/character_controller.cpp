#include "pch.h"

#include "character_controller.h"

#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <JOlt/Physics/Collision/Shape/CapsuleShape.h>

void CharacterController::init()
{
	// create capsule shape
	JPH::Ref<JPH::CapsuleShape> shape = new JPH::CapsuleShape(0.9f, 0.2f);
	JPH::CharacterVirtualSettings settings{};
	settings.mShape = shape;

	m_internal_cc = new JPH::CharacterVirtual(&settings, JPH::to_jph(entity->position), JPH::to_jph(entity->rotation), g_engine.physics->get_system());

	m_internal_cc->SetListener(this);
}

void CharacterController::update()
{
	if (m_kineamtic)
	{
		m_internal_cc->SetPosition(JPH::to_jph(entity->position));
	}
	else
	{
		auto vel = (Vec3(0, 1, 0) * g_engine.physics->get_gravity()) * 1 / 60;

		if (m_internal_cc->GetGroundState() != JPH::CharacterBase::EGroundState::OnGround)
			m_velocity += vel;



		m_internal_cc->SetLinearVelocity(JPH::to_jph(m_velocity));
		g_engine.physics->update_character_virtual(m_internal_cc);
		entity->position = JPH::to_glm(m_internal_cc->GetPosition());

		m_velocity.x = 0;
		m_velocity.z = 0;
	}
}

void CharacterController::set_gravity(bool active)
{
}

void CharacterController::move(Vec3 force)
{
	m_velocity += force;
}

void CharacterController::draw_inspector()
{
	ImGui::Checkbox("Kinematic", &m_kineamtic);
}

void CharacterController::set_kinematic(bool is_kinematic)
{
	m_kineamtic = is_kinematic;
	
	if (m_kineamtic)
	{
		m_velocity = Vec3(0, 0, 0);
	}
}

void CharacterController::OnAdjustBodyVelocity(const JPH::CharacterVirtual* inCharacter, const JPH::Body& inBody2, JPH::Vec3& ioLinearVelocity, JPH::Vec3& ioAngularVelocity)
{
}

bool CharacterController::OnContactValidate(const JPH::CharacterVirtual* inCharacter, const JPH::BodyID& inBodyID2, const JPH::SubShapeID& inSubShapeID2)
{
	return true;
}

void CharacterController::OnContactAdded(const JPH::CharacterVirtual* inCharacter, const JPH::BodyID& inBodyID2, const JPH::SubShapeID& inSubShapeID2, JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::CharacterContactSettings& ioSettings)
{
}

void CharacterController::OnContactSolve(const JPH::CharacterVirtual* inCharacter, const JPH::BodyID& inBodyID2, const JPH::SubShapeID& inSubShapeID2, JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::Vec3Arg inContactVelocity, const JPH::PhysicsMaterial* inContactMaterial, JPH::Vec3Arg inCharacterVelocity, JPH::Vec3& ioNewCharacterVelocity)
{
}
