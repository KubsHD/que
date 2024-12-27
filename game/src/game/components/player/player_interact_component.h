#pragma once

#include <core/ecs.h>

class ControllerComponent;

DEFINE_COMPONENT(PlayerInteractComponent)
public:
	PlayerInteractComponent(ControllerComponent* cc);

	void update() override;

	ControllerComponent* get_interacting_controller() const;
private:
	ControllerComponent* m_controller;
};