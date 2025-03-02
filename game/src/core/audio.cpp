#include "pch.h"

#include "audio.h"

#include <fmod_errors.h>
#include <common/DebugOutput.h>
#include "engine_wrapper.h"





static std::unordered_map<String, std::shared_ptr<Sound>> sounds;





FMOD_VECTOR tmp;
FMOD_VECTOR forward = { 0.0f, 0.0f, -1.0f };
FMOD_VECTOR up = { 0.0f, 1.0f, 0.0f };


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

	fmod_call(m_system->getMasterChannelGroup(&m_master_group));

	set_master_volume(g_engine.config.fmod_default_volume);
}

AudioSystem::~AudioSystem()
{
	for (auto& snd : sounds)
	{
		snd.second->sound->release();
	}
	m_system->release();
}

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
	if (sounds.find(path) != sounds.end())
	{
		return sounds[path].get();
	}

	Sound* snd = new Sound();

	fmod_call(m_system->createSound(path.c_str(), type == SoundType::SPATIAL ? FMOD_3D : FMOD_DEFAULT, nullptr, &snd->sound));

	FMOD_SOUND_TYPE sound_type;
	FMOD_SOUND_FORMAT sound_format;
	int channels, bits;
	float rate;

	snd->sound->getFormat(&sound_type, &sound_format, &channels, &bits);

	if (sound_type == FMOD_SOUND_TYPE_FSB)
	{
		snd->sound->getSubSound(0, &snd->sound);
	}

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

void AudioSystem::set_master_volume(float volume)
{
	m_master_group->setVolume(volume);
}
