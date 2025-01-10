#pragma once

#include <core/ecs.h>

#include <Jolt/Physics/Character/CharacterVirtual.h>

class Component;

namespace JPH
{
	class CharacterVirtual;
}

class CharacterController : public Component, public JPH::CharacterContactListener {
	DEFINE_COMPONENT_BODY(CharacterController)

public:
	void init() override;
	void update() override;
	void set_gravity(bool active);
	bool kinematic = false;
	void move(Vec3 force);

	void draw_inspector() override;

private:
	Vec3 m_velocity = Vec3(0, 0, 0);

	JPH::CharacterVirtual* m_internal_cc = nullptr;

	bool m_draw_debug = false;

	bool m_kinematic = false;
	bool m_gravity_enabled = true;

	float m_character_height = 1.5f;
	float m_character_radius = 0.2f;

	float m_velocity_falloff_speed = 0.5f;
public:
	void set_kinematic(bool is_kinematic);

	void OnAdjustBodyVelocity(const JPH::CharacterVirtual* inCharacter, const JPH::Body& inBody2, JPH::Vec3& ioLinearVelocity, JPH::Vec3& ioAngularVelocity) override;


	bool OnContactValidate(const JPH::CharacterVirtual* inCharacter, const JPH::BodyID& inBodyID2, const JPH::SubShapeID& inSubShapeID2) override;


	void OnContactAdded(const JPH::CharacterVirtual* inCharacter, const JPH::BodyID& inBodyID2, const JPH::SubShapeID& inSubShapeID2, JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::CharacterContactSettings& ioSettings) override;


	void OnContactSolve(const JPH::CharacterVirtual* inCharacter, const JPH::BodyID& inBodyID2, const JPH::SubShapeID& inSubShapeID2, JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::Vec3Arg inContactVelocity, const JPH::PhysicsMaterial* inContactMaterial, JPH::Vec3Arg inCharacterVelocity, JPH::Vec3& ioNewCharacterVelocity) override;

};