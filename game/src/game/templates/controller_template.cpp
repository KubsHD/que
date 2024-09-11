#pragma once
#include "pch.h"
//#include "controller_template.h"
//
//#include <game/components.h>
//#include <core/physics.h>
//#include <game/tags.h>
//
//void game::tmpl::create_controller(entt::registry& reg, Model& controller, int index)
//{
//	const auto controller1 = reg.create();
//	JPH::BodyCreationSettings c_settings(
//		new JPH::SphereShape(0.5f),
//		JPH::RVec3(0, 10.8, 0),
//		JPH::Quat::sIdentity(),
//		JPH::EMotionType::Kinematic,
//		Layers::MOVING);
//
//	reg.emplace<transform_component>(controller1, glm::vec3{ 0.0f,5.0f,0.0f }, glm::quat(1, 0, 0, 0), glm::vec3{ 0.5f, 0.5f, 0.5f });
//	//reg.emplace<physics_component>(controller1, m_physics_system->spawn_body(c_settings, JPH::Vec3(0.7f, -1.0f, 0.1f)));
//	reg.emplace<mesh_component>(controller1, controller);
//	reg.emplace<controller_component>(controller1, index);
//	reg.emplace<main_interacting_controller>(controller1, false);
//}
