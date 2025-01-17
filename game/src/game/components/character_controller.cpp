#include "pch.h"

#include "character_controller.h"

#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <JOlt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <core/profiler.h>

void CharacterController::init()
{
	// create capsule shape
	JPH::Ref<JPH::Shape> shape = JPH::RotatedTranslatedShapeSettings(
		JPH::Vec3(0, 0.5f * m_character_height + m_character_radius, 0),
		JPH::Quat::sIdentity(),
		new JPH::CapsuleShape(0.5f * m_character_height, m_character_radius)).Create().Get();

	JPH::CharacterVirtualSettings settings{};
	settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -m_character_radius); // Accept contacts that touch the lower sphere of the capsule
	settings.mShape = shape;

	m_internal_cc = new JPH::CharacterVirtual(&settings, JPH::to_jph(entity->position), JPH::to_jph(entity->rotation), g_engine.physics->get_system());

	m_internal_cc->SetPosition(JPH::to_jph(entity->position));

	m_internal_cc->SetListener(this);
}

void CharacterController::update()
{
	QUE_PROFILE;

	if (m_kinematic)
	{
		m_internal_cc->SetPosition(JPH::to_jph(entity->position));
	}
	else
	{
		m_internal_cc->UpdateGroundVelocity();

		auto dt = 1 / 72.0f;
		JPH::Vec3 force = JPH::Vec3(0,0,0);

		const auto currentVerticalVelocity =
			m_internal_cc->GetLinearVelocity().Dot(m_internal_cc->GetUp()) * m_internal_cc->GetUp();
		const auto groundVelocity = m_internal_cc->GetGroundVelocity();
		JPH::Vec3 newVelocity;
		bool movingTowardsGround = (currentVerticalVelocity.GetY() - groundVelocity.GetY()) < 0.1f;
		bool characterOnGround =
			m_internal_cc->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround;
		bool useGroundVelocity = movingTowardsGround;

		if (m_gravity_enabled)
		{
			if (characterOnGround && useGroundVelocity) {
				force = groundVelocity;
			}
			else {
				force = currentVerticalVelocity;
			}
		}

		auto gravity = JPH::to_jph(g_engine.physics->get_gravity());

		if (m_gravity_enabled)
			force += gravity * dt;

		if (m_internal_cc->IsSupported() || m_gravity_enabled == false)
		{
			force += JPH::to_jph(m_velocity);
		}
		else {
			JPH::Vec3 currentHorizontalVelocity =
				m_internal_cc->GetLinearVelocity() - currentVerticalVelocity;
			force += currentHorizontalVelocity;
		}

		force += JPH::to_jph(m_force_velocity);

		m_internal_cc->SetLinearVelocity(force);
		entity->position = JPH::to_glm(m_internal_cc->GetPosition());
		m_velocity = Vec3(0);
		m_force_velocity = Vec3(0);
	}

	g_engine.physics->update_character_virtual(m_internal_cc);

#if _DEBUG
	const auto com = m_internal_cc->GetCenterOfMassTransform();
	if (m_draw_debug)
		m_internal_cc->GetShape()->Draw(
			g_engine.physics->get_debug_renderer(), com, JPH::Vec3::sReplicate(1.0f), JPH::Color::sGreen, false, true);
#endif
}

void CharacterController::set_gravity(bool active)
{
	m_gravity_enabled = active;
}

void CharacterController::move(Vec3 force)
{
	m_velocity = force;
}

void CharacterController::move_force(Vec3 force)
{
	m_force_velocity = force;
}

void CharacterController::draw_inspector()
{
	ImGui::Checkbox("Kinematic", &m_kinematic);
	INSPECT_FLOAT("Velocity Falloff Speed", m_velocity_falloff_speed);
}

void CharacterController::set_kinematic(bool is_kinematic)
{
	m_kinematic = is_kinematic;
	
	if (m_kinematic)
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
