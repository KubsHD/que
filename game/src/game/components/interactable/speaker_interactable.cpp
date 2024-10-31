#include "pch.h"

#include "speaker_interactable.h"

void SpeakerInteractableComponent::on_interact()
{
	g_engine.audio->play_sound(m_sound);
}

void SpeakerInteractableComponent::init()
{
	m_sound = g_engine.asset->load_sound("audio/speaker_2.mp3");
}

void SpeakerInteractableComponent::update()
{
}
