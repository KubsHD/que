#include "pch.h"
#include <game/components.h>
#include <core/physics.h>

namespace game {
	namespace tmpl {
		void create_block(entt::registry& reg, PhysicsSystem& psys, glm::vec3 pos, Model& mod)
		{
			const auto ball = reg.create();

			reg.emplace<transform_component>(ball, glm::vec3{ 0.0f,5.0f,0.0f }, glm::quat(1, 0, 0, 0), glm::vec3{ 0.5f, 0.5f, 0.5f });
			reg.emplace<mesh_component>(ball, mod);

			JPH::BodyCreationSettings obj_settings(
				new JPH::BoxShape(JPH::Vec3(0.5f, 0.5f, 0.5f)),
				JPH::RVec3(0, 5, 0),
				JPH::Quat::sIdentity(),
				JPH::EMotionType::Dynamic,
				Layers::MOVING);

			reg.emplace<physics_component>(ball, true, psys.spawn_body(obj_settings, JPH::Vec3(0, -1.0f, 0.0f)));
			reg.emplace<pickupable_block>(ball);
		}
	}
}