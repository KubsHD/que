#include "pch.h"

#include "mesh_component.h"

#include <gfx/mesh.h>
#include <gfx/renderer2.h>


MeshComponent::MeshComponent(Model* model) : m_model(model)
{
}

void MeshComponent::init()
{
	// todo

	assert(m_model->meshes.size() > 0);
	entity->scene->engine.render->register_mesh(this);
}

void MeshComponent::update()
{
}
