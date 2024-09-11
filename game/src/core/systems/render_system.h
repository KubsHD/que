#pragma once

#include <entt/entt.hpp>

#include <core/systems/base_system.h>

class MeshComponent;

class RenderSystem : public System {

public:
	void init() override
	{
	}


	void update() override
	{
	}


	void destroy() override
	{
	}

	void register_model(const MeshComponent* comp);

	void remove_model()
	{

	}

	RenderSystem() = default;
};
