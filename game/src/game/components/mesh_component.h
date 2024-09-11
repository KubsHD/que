#pragma once

#include <core/ecs.h>

class Component;

class MeshComponent : public Component {

public:

	MeshComponent() = default;

	MeshComponent(Model* model);

	void init() override;
	void update() override;

	inline Model* get_model() const { return m_model; }
private:
	Model* m_model;
};