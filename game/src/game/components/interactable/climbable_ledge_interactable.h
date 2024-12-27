#pragma once
#include <core/audio.h>

#include <core/ecs.h>
#include <core/audio.h>

#include <game/components/interactable/base_interactable.h>

class PlayerComponent;
class ControllerComponent;
class PlayerClimbManager;

DEFINE_COMPONENT_DERIVED(ClimbableLedgeInteractable, InteractableComponent)
public:
	void init() override;

	void on_interact(ControllerComponent* cc) override;

	void on_grab(ControllerComponent* cc) override;
	void on_grab_release(ControllerComponent* cc) override;
	
	void update() override;
private:

	PlayerClimbManager* m_player;

	std::shared_ptr<Sound> m_sound;
	bool m_is_playing = false;
};

