#pragma once

#include <entt/entt.hpp>
#include <core/physics.h>
#include <game/components.h>
#include <core/input.h>

namespace game {
	namespace system {
		void update_block_pickup_system(entt::registry& reg, Input& inp);
	}
}