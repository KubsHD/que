#pragma once

#include <entt/entt.hpp>
#include <core/physics.h>
#include <core/input.h>
#include <game/components.h>

namespace game {
	namespace system {
		void update_spawn_block_system(entt::registry& reg);
	}
}