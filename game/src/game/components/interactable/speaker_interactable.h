#pragma once
#include <core/audio.h>

#include <core/ecs.h>
#include <core/audio.h>

#include <game/components/interactable/base_interactable.h>

DEFINE_COMPONENT_DERIVED(SpeakerInteractableComponent, InteractableComponent)
public:

	void init() override;
	void on_interact() override;
	void update() override;

private:

	std::shared_ptr<Sound> m_sound;
	bool m_is_playing = false;
};

