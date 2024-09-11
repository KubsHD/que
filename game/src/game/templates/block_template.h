#include "pch.h"
#include <core/ecs.h>

class PhysicsSystem;

namespace game {
	namespace tmpl {
		Entity* create_block(Scene& scn, glm::vec3 pos, Model* mod, JPH::RefConst<JPH::Shape> customShape);
	}
}