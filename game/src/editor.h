#pragma once

#include <core/types.h>
#include <core/editor/editor_platform.h>

class AudioSystem;

class Editor {
public:
	Editor();
	~Editor();

	void run();

	EditorPlatform* ed_platform;

	entt::registry reg;

	AudioSystem* m_audio_system;

private:
	void editor_ui();
};