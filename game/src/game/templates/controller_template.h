#pragma once

#include <core/physics.h>
#include <core/ecs.h>
#include <gfx/mesh.h>
#include <game/components/player_component.h>

namespace game {
	namespace tmpl {
		void create_controller(Scene& scn, int index, PlayerComponent* pc);
	}
}