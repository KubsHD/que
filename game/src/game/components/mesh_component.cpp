#include "pch.h"

#include "mesh_component.h"

#include <asset/mesh.h>
#include <gfx/renderer.h>


MeshComponent::MeshComponent(Model* model)
{
	m_model = model;
}

void MeshComponent::init()
{
	entity->scene->engine.render->register_mesh(this);
}

void MeshComponent::update()
{
}
