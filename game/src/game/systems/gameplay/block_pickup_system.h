#pragma once

#include <entt/entt.hpp>

class XrInput;
class PhysicsSystem;

namespace game {
	namespace system {
		void update_block_pickup_system(entt::registry& reg, XrInput& inp, PhysicsSystem& psys);
	}
}