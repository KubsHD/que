#pragma once

#include <core/ecs.h>

class Component;

class PhysicsComponent : public Component {
public:
	PhysicsComponent() = default;

	PhysicsComponent(JPH::BodyCreationSettings bodySettings);

	void init() override;
	void update() override;

	bool kinematic = false;

private:
	JPH::BodyID m_body;
};