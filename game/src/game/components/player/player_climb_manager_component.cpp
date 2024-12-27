#include "pch.h"

#include "player_climb_manager_component.h"

#include <core/profiler.h>
#include <core/engine_wrapper.h>

#include <game/components/pickupable_component.h>
#include <game/components/controller_component.h>
#include <game/components/physics_component.h>
#include <game/components/player_component.h>

#include "collision_helpers.h"


void PlayerClimbManager::init()
{
	REQUIRE_COMPONENT(PlayerComponent);

	m_player = entity->get<PlayerComponent>();
}

void PlayerClimbManager::update()
{

}

void PlayerClimbManager::report_controller_attached_to_ledge(ControllerComponent* cc)
{
	// disable player gravity 
// lock controller position
// start moving player realtivly to controller position

	m_controllers_attached_to_ledge.push_back(cc);
	cc->set_frozen(true);

}

void PlayerClimbManager::report_controller_detached_from_ledge(ControllerComponent* cc)
{
	if (std::find(m_controllers_attached_to_ledge.begin(), m_controllers_attached_to_ledge.end(), cc) != m_controllers_attached_to_ledge.end())
	{
		m_controllers_attached_to_ledge.erase(std::remove(m_controllers_attached_to_ledge.begin(), m_controllers_attached_to_ledge.end(), cc), m_controllers_attached_to_ledge.end());
		cc->set_frozen(false);
	}
}
