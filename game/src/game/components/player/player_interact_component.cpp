#include "pch.h"

#include "player_interact_component.h"
#include "collision_helpers.h"

#include <game/components/interactable/base_interactable.h>

void PlayerInteractComponent::update()
{
	if (g_engine.input->get_interaction_button_down())
	{
		auto bodies = g_engine.physics->overlap_sphere(entity->position, 0.0001f);
		auto entities = get_entities_from_collision_result(this->entity->scene, bodies);

		if (entities.size() == 0)
			return;

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
			ent->on_interact();
		}
	}
}