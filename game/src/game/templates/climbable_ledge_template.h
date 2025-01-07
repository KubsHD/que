#pragma once

#include <core/physics.h>
#include <core/ecs.h>
#include <gfx/mesh.h>
#include <game/components/player_component.h>

namespace game {
	namespace tmpl {
		Entity* create_climbable_ledge(Vec3 position, Scene& scn, PlayerComponent* pc);
	}
}