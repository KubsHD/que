#pragma once

#include <core/ecs.h>

class Component;

DEFINE_COMPONENT(PhysicsComponent)
	PhysicsComponent(JPH::BodyCreationSettings bodySettings);

	void init() override;
	void update() override;

	bool kinematic = false;

	JPH::BodyID get_body_id() const;

private:
	JPH::BodyID m_body;
};