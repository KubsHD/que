#pragma once

#include <core/ecs.h>

#include <game/components/controller_component.h>

class PickupableComponent;
class PlayerComponent;
class CharacterController;

class PlayerClimbManager : public Component {
	DEFINE_COMPONENT_BODY(PlayerClimbManager)

	void init() override;
	void update() override;

	void draw_inspector() override;

	void report_controller_attached_to_ledge(ControllerComponent* cc);
	void report_controller_detached_from_ledge(ControllerComponent* cc);
private:
	PlayerComponent* m_player;
	CharacterController* m_cc;

	Vector<ControllerComponent*> m_controllers_attached_to_ledge;

	String m_ledge_grab_sfx = "audio/sfx_ledge_grab.wav";
	String m_ledge_letgo_sfx = "audio/sfx_ledge_letgo.wav";

	std::shared_ptr<Sound> m_ledge_grab_sound;
	std::shared_ptr<Sound> m_ledge_letgo_sound;

	Vec3 m_player_offset = Vec3(0.0f);
	Vec3 m_prev_player_offset = Vec3(0.0f);
};
