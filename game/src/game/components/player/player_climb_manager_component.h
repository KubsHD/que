#pragma once

#include <core/ecs.h>

#include <game/components/controller_component.h>

class PickupableComponent;
class PlayerComponent;

DEFINE_COMPONENT(PlayerClimbManager)
public:
	void init() override;
	void update() override;

	void report_controller_attached_to_ledge(ControllerComponent* cc);
	void report_controller_detached_from_ledge(ControllerComponent* cc);
private:
	PlayerComponent* m_player;

	Vector<ControllerComponent*> m_controllers_attached_to_ledge;
};
