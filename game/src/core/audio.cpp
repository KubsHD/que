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

FMOD_VECTOR tmp;
FMOD_VECTOR forward = { 0.0f, 0.0f, 1.0f };
FMOD_VECTOR up = { 0.0f, 1.0f, 0.0f };

void AudioSystem::update(glm::vec3 listener_position)
{
	m_system->update(); 


	tmp.x = listener_position.x;
	tmp.y = listener_position.y;
	tmp.z = listener_position.z;

	m_system->set3DListenerAttributes(0, &tmp, nullptr, &forward, &up);
}

Sound* AudioSystem::create_sound(String path, SoundType type)
{
	Sound* snd = new Sound();

	fmod_call(m_system->createSound(path.c_str(), type == SoundType::SPATIAL ? FMOD_3D : FMOD_DEFAULT, nullptr, &snd->sound));

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

void AudioSystem::play_sound(std::shared_ptr<Sound> sound)
{
	fmod_call(m_system->playSound(sound->sound, nullptr, false, &sound->chnl));
}
