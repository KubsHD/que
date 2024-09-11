#include "pch.h"

#include "attach_system.h"
#include <core/physics.h>
//#include <game/components.h>
//#include <common/glm_helpers.h>
//#include <core/input.h>
//
//void game::system::on_attach_component_created(entt::registry& reg, entt::entity e)
//{
//	physics_component* pc = reg.try_get<physics_component>(e);
//	if (pc)
//	{
//		//psys.set_motion_type(pc->id, JPH::EMotionType::Kinematic);
//		//pc->enabled = false;
//	}
//
//	// try to run interaction
//	interactable* ic = reg.try_get<interactable>(e);
//	if (ic)
//	{
//		if (!ic->single_use || (ic->single_use && !ic->triggered_once))
//			ic->on_interact(reg);
//	}
//}
//
//void game::system::on_attach_component_destroyed(entt::registry& reg, entt::entity e)
//{
//
//	physics_component* pc = reg.try_get<physics_component>(e);
//	if (pc)
//	{
//		//psys.add_velocity(pc->id, cc.vel * 10);
//		//psys.set_motion_type(pc->id, JPH::EMotionType::Dynamic);
//		//pc->enabled = true;
//	}
//}
//
//void game::system::update_attach_system(entt::registry& reg)
//{
//	for (const auto& [e, tc, ac] : reg.view<transform_component, attach_component>().each())
//	{
//		auto tc_parent = reg.get<transform_component>(ac.parent_entity);
//		tc.position = tc_parent.position;
//
//		tc.rotation = tc_parent.rotation * tc.rotation2;
//	}
//}
//
//void game::system::on_speaker_interact(entt::registry& reg, entt::entity e)
//{
//	engine eng = reg.ctx().get<engine>();
//	eng.audio.play_sound("data/audio/speaker_1.mp3");
//}