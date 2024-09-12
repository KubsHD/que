#pragma once

#include <core/ecs.h>

DEFINE_COMPONENT(PickupableComponent)
	void init() override;
	void update() override;

	virtual void on_pickup();
	virtual void on_drop();

	bool is_held = false;
};