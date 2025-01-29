#pragma once

#include <core/types.h>
#include <fmod.hpp>

struct Sound {
	String path;
	FMOD::Sound* sound;
	FMOD::Channel* chnl;
};

enum class SoundType {
	FLAT,
	SPATIAL,
	MAX
};

class AudioSystem {
public:
	AudioSystem();
	~AudioSystem();

	void update(glm::vec3 listener_position);

	Sound* create_sound(String path, SoundType type = SoundType::FLAT);

	void play_sound(Sound& sound);
	void play_sound(std::shared_ptr<Sound> sound);
	void play_sound(String path);

	void set_master_volume(float volume);

	inline FMOD::System* get_internal_system() 
	{
		return m_system;
	};
private:
	FMOD::System* m_system;

	FMOD::ChannelGroup* m_master_group;
};