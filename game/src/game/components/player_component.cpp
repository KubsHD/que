#include "pch.h"

#include "player_component.h"
#include <common/glm_helpers.h>
#include <entt/entt.hpp>

#include <gfx/renderer2.h>

void PlayerComponent::init()
{
}

void PlayerComponent::update()
{
	auto vel = g_engine.input->get_movement_input();
	m_forward = glm::normalize(glm::to_glm(pose.orientation)) * glm::vec3(0.0f, 0.0f, -1.0f);
	auto right = glm::normalize(glm::to_glm(pose.orientation)) * glm::vec3(1.0f, 0.0f, 0.0f);;
	auto forward = m_forward;


	entity->position = entity->position + (vel.y * forward * m_speed) + vel.x * right * m_speed;

	g_engine.render->set_camera_position(entity->position);
}

