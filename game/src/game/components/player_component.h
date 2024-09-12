#pragma once

#include <core/ecs.h>

#include <openxr/openxr.h>

class PlayerComponent : public Component {

public:

	PlayerComponent() = default;

	void init() override;
	void update() override;

	XrPosef pose;

	float viewHeightM = 1.5f;
private:
	glm::vec3 m_forward;
	float m_speed = 0.1f;
};