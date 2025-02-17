#pragma once

#include <core/ecs.h>
#include <core/audio.h>

class ControllerComponent;

class InteractableComponent : public Component
{
	DEFINE_COMPONENT_BODY(InteractableComponent);
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