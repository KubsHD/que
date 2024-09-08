#pragma once

#include <entt/entt.hpp>
#include <core/physics.h>

namespace game {
	namespace system {
		void update_physics_system(entt::registry& reg, PhysicsSystem& sys);
	}
}