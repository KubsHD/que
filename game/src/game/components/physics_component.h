#pragma once

#include <core/ecs.h>

enum class ColliderType{
	Static,
	Dynamic
};

class PhysicsComponent : public Component {
	DEFINE_COMPONENT_BODY(PhysicsComponent)

	PhysicsComponent(JPH::BodyCreationSettings bodySettings);

	// static collider
	PhysicsComponent(Mesh& mesh);

	void init() override;
	void update() override;

	bool kinematic = false;

	JPH::BodyID get_body_id() const;

private:
	JPH::BodyID m_body;

	ColliderType m_collider_type;
	Mesh m_mesh;
};