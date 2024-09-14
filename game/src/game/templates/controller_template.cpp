#pragma once
#include "pch.h"

#include "controller_template.h"

#include <core/physics.h>
#include <game/components/mesh_component.h>
#include <game/components/controller_component.h>
#include <game/components/player/player_pickup_component.h>
#include <game/components/player/player_interact_component.h>

void game::tmpl::create_controller(Scene& scn, Model& controller, int index, PlayerComponent* pc)
{
	const auto controller1 = scn.create("controller_" + index);
	JPH::BodyCreationSettings c_settings(
		new JPH::SphereShape(0.5f),
		JPH::RVec3(0, 10.8, 0),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Kinematic,
		Layers::MOVING);
	
	controller1->add<MeshComponent>(MeshComponent(&controller));
	controller1->scale = Vec3(0.01f);

	controller1->add<ControllerComponent>(ControllerComponent(index, pc));
	controller1->add<PlayerPickupComponent>();
	controller1->add<PlayerInteractComponent>();

	//reg.emplace<transform_component>(controller1, glm::vec3{ 0.0f,5.0f,0.0f }, glm::quat(1, 0, 0, 0), glm::vec3{ 0.5f, 0.5f, 0.5f });
	////reg.emplace<physics_component>(controller1, m_physics_system->spawn_body(c_settings, JPH::Vec3(0.7f, -1.0f, 0.1f)));
	//reg.emplace<controller_component>(controller1, index);
	//reg.emplace<main_interacting_controller>(controller1, false);
}
