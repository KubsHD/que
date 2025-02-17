#pragma once

#include <asset/asset_manager.h>
#include <core/audio.h>
#include <core/physics.h>
#include <core/xr/xr_input.h>

class Renderer2;

struct engine_config {
	bool r_show_debug = true;
	bool r_show_imgui = false;
	float fmod_default_volume = 0.3f;	
};

struct engine_wrapper {
	AudioSystem* audio;
	AssetManager* asset;
	PhysicsSystem* physics;
	XrInput* input;
	Renderer2* render;
	entt::registry* reg;
	engine_config config;
};

extern engine_wrapper g_engine;
