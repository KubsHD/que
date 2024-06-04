#pragma once
#include <game/components.h>
#include <core/physics.h>
#include <game/tags.h>

namespace game {
	namespace tmpl {
		void create_controller(entt::registry& reg, Model& controller, int index);
	}
}