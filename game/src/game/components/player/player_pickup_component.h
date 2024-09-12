#pragma once

#include <core/ecs.h>

class PickupableComponent;
class ControllerComponent;

DEFINE_COMPONENT(PlayerPickupComponent)
public:
	void init() override;
	void update() override;

	void try_pickup();

	void try_release();

private:
	Entity* m_pickuped_object;
	ControllerComponent* m_controller;
};
