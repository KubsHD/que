#pragma once

#include <entt/entt.hpp>
#include <core/physics.h>
#include <game/components.h>
#include <common/glm_helpers.h>
#include <core/input.h>

namespace game {
	namespace system {
		void update_game_system(entt::registry& reg);
	}
}