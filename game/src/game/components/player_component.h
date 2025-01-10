#pragma once

#include <core/ecs.h>

#include <openxr/openxr.h>

class CharacterController;

class PlayerComponent : public Component
{
	DEFINE_COMPONENT_BODY(PlayerComponent)

	PlayerComponent(CharacterController* cc) : m_cc(cc) {};

	void init() override;
	void update() override;

	void draw_inspector() override;

	XrPosef pose;

	float viewHeightM = 1.0f;

private:

	CharacterController* m_cc = nullptr;

	glm::vec3 m_forward = glm::vec3(0,0,1);
	float m_speed = 1.0f;
};