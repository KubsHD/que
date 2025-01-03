#include "pch.h"

#include "player_component.h"
#include <common/glm_helpers.h>
#include <entt/entt.hpp>

#include <game/components/character_controller.h>

#include <gfx/renderer2.h>
#include <common/DebugOutput.h>

void PlayerComponent::init()
{
}

void PlayerComponent::update()
{
	auto vel = g_engine.input->get_movement_input();
	m_forward = /*glm::normalize(glm::to_glm(pose.orientation)) * */glm::vec3(0.0f, 0.0f, -1.0f);
	auto right = /*glm::normalize(glm::to_glm(pose.orientation)) **/ glm::vec3(1.0f, 0.0f, 0.0f);;
	auto forward = m_forward;


	//entity->position = entity->position + (vel.y * forward * m_speed) + vel.x * right * m_speed;

	Vec3 move_dir = (vel.x * right * m_speed) + (vel.y * forward * m_speed);
	move_dir.y = 0;

	LOG_INFO("vel: " + std::to_string(move_dir.x) + " " + std::to_string(move_dir.y) + " " + std::to_string(move_dir.z));

	m_cc->move(move_dir);

	g_engine.render->set_camera_position(entity->position + Vec3(0,viewHeightM, 0));
}

void PlayerComponent::draw_inspector()
{
	ImGui::DragFloat("View Height", &viewHeightM);
	ImGui::DragFloat("Speed", &m_speed);
}