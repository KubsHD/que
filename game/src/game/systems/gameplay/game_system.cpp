#include "game_system.h"



void game::system::update_game_system(entt::registry& reg)
{
	auto c = reg.view<native_script_component>();
	for (const auto&& [e, nsc] : c.each())
	{
		nsc.update();
	}
}
