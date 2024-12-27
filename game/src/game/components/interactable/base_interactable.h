#pragma once

#include <core/ecs.h>
#include <core/audio.h>

class ControllerComponent;

DEFINE_COMPONENT(InteractableComponent)
public:
	virtual void on_interact(ControllerComponent* cc) = 0;
	virtual void on_interact_hold(ControllerComponent* cc);
	virtual void on_interact_release(ControllerComponent* cc);

	virtual void on_grab(ControllerComponent* cc);
	virtual void on_grab_hold(ControllerComponent* cc);
	virtual void on_grab_release(ControllerComponent* cc);

private:
	INSPECT(bool, m_can_interact);
};