#pragma once

#include <asset/asset_manager.h>
#include <core/audio.h>
#include <core/physics.h>
#include <core/xr/xr_input.h>

class Renderer2;

struct engine_wrapper {
	AudioSystem* audio;
	AssetManager* asset;
	PhysicsSystem* physics;
	XrInput* input;
	Renderer2* render;
	entt::registry* reg;
};

extern engine_wrapper g_engine;
