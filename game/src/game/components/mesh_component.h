#pragma once

#include <core/ecs.h>

class Component;

class MeshComponent : public Component {
	DEFINE_COMPONENT_BODY(MeshComponent)

	MeshComponent(Model* model);
	MeshComponent(String path);

	void init() override;
	void update() override;
	void destroy() override;

	inline Model* get_model() { return &m_model; }
private:
	Model m_model;
};