#include "pch.h"

#include "audio.h"

#include <fmod_errors.h>
#include <common/DebugOutput.h>

void fmod_call(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		XR_TUT_LOG_ERROR("FMOD error! " << FMOD_ErrorString(result));
	}
}

AudioSystem::AudioSystem()
{
	fmod_call(FMOD::System_Create(&m_system));
	fmod_call(m_system->init(512, FMOD_INIT_NORMAL, 0));
}

AudioSystem::~AudioSystem()
{

}

void AudioSystem::update()
{
	m_system->update(); 
}

Sound* AudioSystem::create_sound(String path)
{
	Sound* snd = new Sound();

	fmod_call(m_system->createSound(path.c_str(), FMOD_DEFAULT, nullptr, &snd->sound));

	return snd;
}

void AudioSystem::play_sound(Sound& sound)
{
	fmod_call(m_system->playSound(sound.sound, nullptr, false, &sound.chnl));
}

void AudioSystem::play_sound(String path)
{
	play_sound(*create_sound(path));
}
