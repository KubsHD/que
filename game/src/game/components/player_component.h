#pragma once

#include <core/ecs.h>

#include <openxr/openxr.h>

DEFINE_COMPONENT(PlayerComponent)
	void init() override;
	void update() override;

	XrPosef pose;

	float viewHeightM = 1.0f;
private:
	glm::vec3 m_forward = glm::vec3(0,0,1);
	float m_speed = 0.1f;
};