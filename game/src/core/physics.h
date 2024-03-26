#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <entt/entt.hpp>

class PhysicsWorld {
public:
	PhysicsWorld();

	void update(float dt, entt::registry& reg);

private:
	JPH::PhysicsSystem m_system;
};