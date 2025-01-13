#pragma once

#include <core/ecs.h>

#include <openxr/openxr.h>
#include <core/components/components.h>

class CharacterController;

class LightComponent : public Component
{
	DEFINE_COMPONENT_BODY(LightComponent)

	LightComponent(LightType type, glm::vec3 color, float intensity, float range);;

	void init() override;
	void draw_inspector() override;
	
	void update() override;

	void destroy() override;

private:
	LightType type = LightType::Spot;

	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0f;
	float range = 10.0f;

	// spot
	float angle = 45.0f;
};