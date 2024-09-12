#pragma once

#include <core/ecs.h>

class Component;
class PlayerComponent;

class ControllerComponent : public Component {

public:

	ControllerComponent() = default;

	ControllerComponent(int index, PlayerComponent* pc);

	void init() override;
	void update() override;
private:
	int m_index;

	PlayerComponent* m_pc;
};