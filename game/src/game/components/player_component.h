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
	float m_speed = 1.0f;

	float m_footstep_timer = 0.0f;
	float m_footstep_interval = 6.5f;
	std::vector<std::shared_ptr<Sound>> m_footstep_sounds;
};