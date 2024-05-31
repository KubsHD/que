#include "pch.h"

#include "attach_system.h"

void game::system::update_attach_system(entt::registry& reg)
{
	for (const auto& [e, tc, ac] : reg.view<transform_component, attach_component>().each())
	{
		auto tc_parent = reg.get<transform_component>(ac.parent_entity);

		tc.position = tc_parent.position;
	}
}
