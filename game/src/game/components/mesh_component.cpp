#include "pch.h"

#include "mesh_component.h"

#include <gfx/mesh.h>
#include <gfx/renderer2.h>


MeshComponent::MeshComponent(Model* model) : m_model(model)
{
	assert(model != nullptr);
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

void MeshComponent::destroy()
{
	entity->scene->engine.render->unregister_mesh(this);
}
