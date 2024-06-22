#include "pch.h"
#include <game/components.h>

class PhysicsSystem;

namespace game {
	namespace tmpl {
		entt::entity create_block(entt::registry& reg, PhysicsSystem& psys, glm::vec3 pos, Model& mod);
	}
}