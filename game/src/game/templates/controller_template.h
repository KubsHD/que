#pragma once

#include <core/physics.h>
#include <core/ecs.h>
#include <asset/model.h>
#include <game/components/player_component.h>

namespace game {
	namespace tmpl {
		void create_controller(Scene& scn, Model& controller, int index, PlayerComponent* pc);
	}
}