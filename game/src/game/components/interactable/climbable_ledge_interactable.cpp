#include "pch.h"

#include "climbable_ledge_interactable.h"

#include <game/components/player_component.h>
#include <game/components/character_controller.h>
#include <game/components/controller_component.h>
#include <game/components/player/player_climb_manager_component.h>

void ClimbableLedgeInteractable::init()
{
	m_player = entity->scene->get("player")->get<PlayerClimbManager>();
}

void ClimbableLedgeInteractable::update()
{
}

void ClimbableLedgeInteractable::on_grab(ControllerComponent* cc)
{
	m_player->report_controller_attached_to_ledge(cc);
	// disable player gravity 
	// lock controller position
	// start moving player realtivly to controller position
}

void ClimbableLedgeInteractable::on_grab_release(ControllerComponent* cc)
{
	m_player->report_controller_detached_from_ledge(cc);
	// unlock controller position
	// stop moving player realtivly to controller position
	// enable player gravity
}

void ClimbableLedgeInteractable::on_interact(ControllerComponent* cc)
{

}

