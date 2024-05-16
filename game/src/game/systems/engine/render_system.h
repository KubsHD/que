#pragma once

#include <entt/entt.hpp>
#include <core/physics.h>
#include <game/components.h>
#include <common/glm_helpers.h>
#include <core/input.h>

namespace game {
	namespace system {
		void update_render_system(entt::registry& reg, Input& input, PhysicsSystem& psys, float viewHeight)
		{
			auto modelsToRender = reg.view<transform_component, mesh_component>();
			for (const auto&& [e, tc, mc] : modelsToRender.each())
			{
				//render_model(tc.position, tc.scale, tc.rotation, mc.model);
			};
		}
	}
}