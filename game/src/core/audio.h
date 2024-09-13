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
	void play_sound(Sound& sound);
	void play_sound(String path);

	inline FMOD::System* get_internal_system() 
	{
		return m_system;
	};
private:
	FMOD::System* m_system;
};