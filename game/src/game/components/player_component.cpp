#include "pch.h"

#include "player_component.h"
#include <common/glm_helpers.h>
#include <entt/entt.hpp>

#include <game/components/character_controller.h>

#include <gfx/renderer2.h>
#include <common/DebugOutput.h>

void PlayerComponent::init()
{
	m_footstep_sounds.push_back(g_engine.asset->load_sound("audio/sfx_footstep_1.wav"));
	m_footstep_sounds.push_back(g_engine.asset->load_sound("audio/sfx_footstep_2.wav"));
	m_footstep_sounds.push_back(g_engine.asset->load_sound("audio/sfx_footstep_3.wav"));
	m_footstep_sounds.push_back(g_engine.asset->load_sound("audio/sfx_footstep_4.wav"));
}

void PlayerComponent::update()
{
	auto vel = g_engine.input->get_movement_input();

	auto right = g_engine.input->get_headset_right();
	auto forward = g_engine.input->get_headset_forward();

	Vec3 move_dir = vel.y * forward * m_speed + vel.x * right * m_speed;
	move_dir.y = 0;

	if (move_dir.x != 0 || move_dir.z != 0)
	{
		if (m_footstep_timer > m_footstep_interval)
		{
			m_footstep_timer = 0.0f;
			auto sound = m_footstep_sounds[rand() % m_footstep_sounds.size()];

			g_engine.audio->play_sound(sound);
		}
		else
		{
			m_footstep_timer += 0.16f;
		}
	}

	m_cc->move(move_dir);

	g_engine.render->set_camera_position(entity->position + Vec3(0,viewHeightM, 0));
}

void PlayerComponent::draw_inspector()
{
	ImGui::DragFloat("View Height", &viewHeightM);
	ImGui::DragFloat("Speed", &m_speed);
	ImGui::DragFloat("Footstep Interval", &m_footstep_interval);
}