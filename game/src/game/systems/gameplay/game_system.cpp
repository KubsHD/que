#include "pch.h"

#include "game_system.h"

#include <core/physics.h>
#include <game/components.h>
#include <common/glm_helpers.h>
#include <core/input.h>

void game::system::update_game_system(entt::registry& reg)
{
	auto c = reg.view<native_script_component>();
	for (const auto&& [e, nsc] : c.each())
	{
		nsc.update();
	}
}
