#pragma once

#include <entt/entt.hpp>
#include <core/physics.h>
#include <common/glm_helpers.h>
#include <core/xr/xr_input.h>

namespace game {
	namespace system {
		void update_controller_system(entt::registry& reg, XrInput& input, PhysicsSystem& psys, float viewHeight, glm::vec3 playerPos);
	}
}