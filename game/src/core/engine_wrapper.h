#pragma once

#include <core/asset.h>
#include <core/audio.h>
#include <core/physics.h>
#include <core/systems/render_system.h>

struct engine_wrapper {
	AudioSystem& audio;
	AssetSystem& asset;
	PhysicsSystem& physics;
	RenderSystem& render;
};