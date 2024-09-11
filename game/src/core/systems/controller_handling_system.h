#pragma once

#include <entt/entt.hpp>
#include <core/physics.h>
#include <common/glm_helpers.h>
#include <core/input.h>

namespace game {
	namespace system {
		void update_controller_system(entt::registry& reg, Input& input, PhysicsSystem& psys, float viewHeight, glm::vec3 playerPos);
	}
}