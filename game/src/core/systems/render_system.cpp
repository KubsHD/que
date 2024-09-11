#include "pch.h"

#include "render_system.h"

#include <entt/entt.hpp>
#include <game/components/mesh_component.h>
#include <core/components/components.h>


void RenderSystem::register_model(const MeshComponent* comp)
{
	m_reg.emplace<core_mesh_component>(comp->entity->internal_entity, comp->get_model());
}
