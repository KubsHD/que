#pragma once

#include <core/ecs.h>

class Component;
class PlayerComponent;

DEFINE_COMPONENT(ControllerComponent)
	ControllerComponent(int index, PlayerComponent* pc);

	void init() override;
	void update() override;

	int index;
private:

	PlayerComponent* m_pc;
};