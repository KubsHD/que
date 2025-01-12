#include "pch.h"

#include "light_component.h"
#include <common/glm_helpers.h>
#include <entt/entt.hpp>

#include <game/components/character_controller.h>

#include <gfx/renderer2.h>
#include <common/DebugOutput.h>


// todo: make lights dynamic

LightComponent::LightComponent(LightType type, glm::vec3 color, float intensity, float range) : type(type), color(color), intensity(intensity), range(range)
{

}

void LightComponent::init()
{

	if (type == LightType::Spot)
	{
		direction = entity->position + glm::vec3(0, -10, 0);
		angle = glm::radians(25.0f);
	}

	g_engine.reg->emplace<core_light_component>(this->entity->internal_entity, type, color, intensity, range, direction, angle);
}

void LightComponent::draw_inspector()
{

}

void LightComponent::update()
{
	// update light

	auto lights = g_engine.reg->view<core_light_component>();

	for (auto [e, light] : lights.each())
	{
		if (e == this->entity->internal_entity)
		{
			light.color = this->color;
			light.intensity = this->intensity;
			light.range = this->range;
			light.direction = this->direction;
			light.angle = this->angle;
		}
	};
}
