#pragma once

#include <core/ecs.h>

class Component;
class PlayerComponent;

DEFINE_COMPONENT(ControllerComponent)
	ControllerComponent(int index, PlayerComponent* pc);

	void init() override;
	void update() override;

	Vec3 get_velocity() { return m_velocity; }

	inline void set_frozen(bool frozen)
	{
		m_frozen = frozen;
	}

	int index;
private:
	Vec3 last_pos;

	Vec3 m_velocity;
	PlayerComponent* m_pc;

	bool m_frozen = false;
};