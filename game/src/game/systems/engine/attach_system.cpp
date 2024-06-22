#include "pch.h"

#include "attach_system.h"
#include <core/physics.h>
#include <game/components.h>
#include <common/glm_helpers.h>
#include <core/input.h>

void game::system::update_attach_system(entt::registry& reg)
{
	for (const auto& [e, tc, ac] : reg.view<transform_component, attach_component>().each())
	{
		auto tc_parent = reg.get<transform_component>(ac.parent_entity);
		tc.position = tc_parent.position;
	}
}
