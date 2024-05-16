#pragma once

#include <entt/entt.hpp>
#include <core/physics.h>
#include <game/components.h>

namespace game {
	namespace system {
		void update_physics_system(entt::registry& reg, PhysicsSystem& sys);
	}
}