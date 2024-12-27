#pragma once

#include <core/ecs.h>

class Component;

namespace JPH
{
	class CharacterVirtual;
}

DEFINE_COMPONENT(CharacterController)
	void init() override;
	void update() override;
	void set_gravity(bool active);
	bool kinematic = false;
private:
	JPH::CharacterVirtual* m_internal_cc = nullptr;
};