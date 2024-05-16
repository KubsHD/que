#include "block_spawn_system.h"



void game::system::update_spawn_block_system(entt::registry& reg)
{
	auto entities = reg.view<transform_component, new_block_spawn_point_component>();
	for (const auto&& [e, tc] : entities.each())
	{

	};
}
