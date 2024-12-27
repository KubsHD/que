#include "pch.h"

#include "player_interact_component.h"
#include "collision_helpers.h"

#include <game/components/interactable/base_interactable.h>

#include <game/components/controller_component.h>

void PlayerInteractComponent::update()
{
	auto bodies = g_engine.physics->overlap_sphere(entity->position, 0.01f);
	auto entities = get_entities_from_collision_result(this->entity->scene, bodies);
	if (entities.size() == 0)
		return;


	if (g_engine.input->get_interaction_button())
	{
		// find an entity that has pickupable component
		InteractableComponent* ent = nullptr;
		for (auto& e : entities)
		{
			auto pb = e->get<InteractableComponent>();
			if (pb)
			{
				ent = pb;
				break;
			}
		}
		if (ent != nullptr)
		{
			ent->on_interact_hold(m_controller);
		}
	}

	if (g_engine.input->get_interaction_button_down())
	{
		// find an entity that has pickupable component
		InteractableComponent* ent = nullptr;
		for (auto& e : entities)
		{
			auto pb = e->get<InteractableComponent>();
			if (pb)
			{
				ent = pb;
				break;
			}
		}

		if (ent != nullptr)
		{
			ent->on_interact(m_controller);
		}
	}

	if (g_engine.input->get_interaction_button_up())
	{
		// find an entity that has pickupable component
		InteractableComponent* ent = nullptr;
		for (auto& e : entities)
		{
			auto pb = e->get<InteractableComponent>();
			if (pb)
			{
				ent = pb;
				break;
			}
		}
		if (ent != nullptr)
		{
			ent->on_interact_release(m_controller);
		}
	}

	if (g_engine.input->get_grab_button((ControllerType)m_controller->index))
	{
		InteractableComponent* ent = nullptr;
		for (auto& e : entities)
		{
			auto pb = e->get<InteractableComponent>();
			if (pb)
			{
				ent = pb;
				break;
			}
		}
		if (ent != nullptr)
		{
			ent->on_grab_hold(m_controller);
		}
	}

	if (g_engine.input->get_grab_button_down((ControllerType)m_controller->index))
	{
		InteractableComponent* ent = nullptr;
		for (auto& e : entities)
		{
			auto pb = e->get<InteractableComponent>();
			if (pb)
			{
				ent = pb;
				break;
			}
		}
		if (ent != nullptr)
		{
			ent->on_grab(m_controller);
		}
	}

	if (g_engine.input->get_grab_button_up((ControllerType)m_controller->index))
	{
		InteractableComponent* ent = nullptr;
		for (auto& e : entities)
		{
			auto pb = e->get<InteractableComponent>();
			if (pb)
			{
				ent = pb;
				break;
			}
		}
		if (ent != nullptr)
		{
			ent->on_grab_release(m_controller);
		}
	}
}

PlayerInteractComponent::PlayerInteractComponent(ControllerComponent* cc)
{
	m_controller = cc;
}

ControllerComponent* PlayerInteractComponent::get_interacting_controller() const
{
	return m_controller;
}
