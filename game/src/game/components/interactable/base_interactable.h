#pragma once

#include <core/ecs.h>
#include <core/audio.h>

DEFINE_COMPONENT(InteractableComponent)
public:
	virtual void on_interact() = 0;

private:
	INSPECT(bool, m_can_interact);
};