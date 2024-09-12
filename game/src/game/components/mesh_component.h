#pragma once

#include <core/ecs.h>

class Component;

DEFINE_COMPONENT(MeshComponent)
	MeshComponent(Model* model);

	void init() override;
	void update() override;

	inline Model* get_model() const { return m_model; }
private:
	Model* m_model;
};