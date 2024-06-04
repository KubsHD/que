#include "pch.h"

#include "physics_system.h"
#include <game/components.h>


void game::system::update_physics_system(entt::registry& reg, PhysicsSystem& sys)
{
	auto physEntities = reg.view<transform_component, physics_component>();
	for (const auto&& [e, tc, pc] : physEntities.each())
	{
		auto type = sys.get_body_type(pc.id);
		if (type == JPH::EMotionType::Dynamic && pc.enabled)
		{
			tc.position = sys.get_body_position(pc.id);
			tc.rotation = sys.get_body_rotation(pc.id);
		}
		else
		{
			sys.set_body_position(pc.id, tc.position);
			//sys.set_body_rotation(pc.id, tc.rotation);
		}
	};
}
