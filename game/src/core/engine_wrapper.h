#pragma once

#include <core/asset.h>
#include <core/audio.h>
#include <core/physics.h>
#include <core/input.h>

class Renderer;

struct engine_wrapper {
	AudioSystem* audio;
	AssetSystem* asset;
	PhysicsSystem* physics;
	Input* input;
	Renderer* render;
	entt::registry* reg;
};

extern engine_wrapper g_engine;
