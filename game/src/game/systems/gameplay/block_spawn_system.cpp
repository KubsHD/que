#include "pch.h"

#include "block_spawn_system.h"

#include <core/physics.h>
#include <core/input.h>
#include <game/components.h>


static float timer = 0.0f;

void game::system::update_spawn_block_system(entt::registry& reg)
{
	timer += 0.1f;
}
