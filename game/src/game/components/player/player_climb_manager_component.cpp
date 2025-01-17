#include "pch.h"

#include "player_climb_manager_component.h"

#include <core/profiler.h>
#include <core/engine_wrapper.h>

#include <game/components/pickupable_component.h>
#include <game/components/controller_component.h>
#include <game/components/physics_component.h>
#include <game/components/player_component.h>
#include <game/components/character_controller.h>

#include "collision_helpers.h"
#include <common/DebugOutput.h>


void PlayerClimbManager::init()
{
	REQUIRE_COMPONENT(PlayerComponent);

	m_player = entity->get<PlayerComponent>();
	m_cc = entity->get<CharacterController>();
}


void PlayerClimbManager::update()
{
	m_player_offset = Vec3(0.0f);


	if (!m_controllers_attached_to_ledge.empty())
	{
		for (auto& cc : m_controllers_attached_to_ledge)
		{
			m_player_offset += cc->get_velocity();
		}

		m_prev_player_offset = m_player_offset;

		m_cc->move(m_player_offset);

		m_player_offset = glm::lerp(m_player_offset, Vec3(0.0f), 0.5f);

	}
}

void PlayerClimbManager::draw_inspector()
{
	ImGui::Text("Controllers attached to ledge: %d", m_controllers_attached_to_ledge.size());
	ImGui::Text("Player offset: %f %f %f", m_prev_player_offset.x, m_prev_player_offset.y, m_prev_player_offset.z);
}

void PlayerClimbManager::report_controller_attached_to_ledge(ControllerComponent* cc)
{
	// disable player gravity 
// lock controller position
// start moving player realtivly to controller position

	m_controllers_attached_to_ledge.push_back(cc);
	cc->set_frozen(true);
	m_cc->set_gravity(false);

}

void PlayerClimbManager::report_controller_detached_from_ledge(ControllerComponent* cc)
{
	if (std::find(m_controllers_attached_to_ledge.begin(), m_controllers_attached_to_ledge.end(), cc) != m_controllers_attached_to_ledge.end())
	{
		m_controllers_attached_to_ledge.erase(std::remove(m_controllers_attached_to_ledge.begin(), m_controllers_attached_to_ledge.end(), cc), m_controllers_attached_to_ledge.end());
		cc->set_frozen(false);
	}

	if (m_controllers_attached_to_ledge.empty())
	{
		m_cc->set_gravity(true);
		m_cc->move_force(m_prev_player_offset * 0.5f);
	}
}
