#pragma once

#include <core/types.h>
#include <fmod.hpp>

struct Sound {
	String path;
	FMOD::Sound* sound;
	FMOD::Channel* chnl;
};

class AudioSystem {
public:
	AudioSystem();
	~AudioSystem();

	void update();

	Sound* create_sound(String path);
	void play_sound(std::shared_ptr<Sound> sound);

	inline FMOD::System* get_internal_system() 
	{
		return m_system;
	};
private:
	FMOD::System* m_system;
};