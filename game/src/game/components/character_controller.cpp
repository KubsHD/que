#include "pch.h"

#include "character_controller.h"

#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <JOlt/Physics/Collision/Shape/CapsuleShape.h>

void CharacterController::init()
{
	// create capsule shape
	JPH::Ref<JPH::CapsuleShape> shape = new JPH::CapsuleShape(0.5f, 1.0f);
	JPH::CharacterVirtualSettings settings{};
	settings.mShape = shape;

	m_internal_cc = new JPH::CharacterVirtual(&settings, JPH::to_jph(entity->position), JPH::to_jph(entity->rotation), g_engine.physics->get_system());
}

void CharacterController::update()
{
	g_engine.physics->update_character_virtual(m_internal_cc);
}

void CharacterController::set_gravity(bool active)
{
	throw std::logic_error("The method or operation is not implemented.");
}
