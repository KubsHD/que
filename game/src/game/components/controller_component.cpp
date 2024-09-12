#include "pch.h"

#include "controller_component.h"
#include <common/glm_helpers.h>

#include <game/components/player_component.h>

ControllerComponent::ControllerComponent(int index, PlayerComponent* pc) : m_index(index), m_pc(pc)
{

}

void ControllerComponent::init()
{

}

void ControllerComponent::update()
{
	const auto& poses = g_engine.input->get_controller_poses();
	auto pose = poses[m_index];

	glm::vec3 target_pos = glm::to_glm(pose.position);
	target_pos += glm::vec3{ m_pc->entity->position.x, m_pc->viewHeightM, m_pc->entity->position.z };
	glm::quat xr_source_rotation = glm::to_glm(pose.orientation);
	glm::quat rot = glm::rotate(xr_source_rotation, glm::radians(180.0f), glm::vec3(0, 1, 0));

	this->entity->position = target_pos;
	this->entity->rotation = rot;
}
