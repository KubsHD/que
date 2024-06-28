#include "pch.h"

#include "controller_handling_system.h"

#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>
#include <game/components.h>
#include <common/DebugOutput.h>

void game::system::update_controller_system(entt::registry& reg, Input& input, PhysicsSystem& psys, float viewHeight, glm::vec3 playerPos)
{
	auto controllerObjects = reg.view<transform_component, mesh_component, controller_component>();
	for (const auto&& [e, tc, mc, cc] : controllerObjects.each())
	{
		const auto& poses = input.get_controller_poses();
		auto pose = poses[cc.index];

		glm::vec3 target_pos = glm::to_glm(pose.position);
		target_pos += glm::vec3{ playerPos.x, viewHeight, playerPos.z };
		glm::quat xr_source_rotation = glm::to_glm(pose.orientation);
		glm::quat rot = glm::rotate(xr_source_rotation, glm::radians(180.0f), glm::vec3(0, 1, 0));

		cc.vel = cc.last_pos - target_pos;

		//XR_TUT_LOG(cc.vel.x << " " << cc.vel.y << " " << cc.vel.z);

		//psys.set_body_position(pc.id, target_pos);

		tc.position = target_pos;
		tc.scale = { 0.01f, 0.01f, 0.01f };
		tc.rotation = rot;

		cc.last_pos = target_pos;
	};
}
