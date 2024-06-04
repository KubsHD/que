#pragma once

#include <entt/entt.hpp>

class Input;
class PhysicsSystem;

namespace game {
	namespace system {
		void update_block_pickup_system(entt::registry& reg, Input& inp, PhysicsSystem& psys);
	}
}