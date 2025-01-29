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
		angle = glm::radians(38.0f);
	}

	g_engine.reg->emplace<core_light_component>(this->entity->internal_entity, type, color, intensity, range, glm::vec3(0,0,0), angle);
}

void LightComponent::draw_inspector()
{
	ImGui::ColorEdit3("Color", glm::value_ptr(color));
	ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1000.0f);
	ImGui::SliderFloat("Range", &range, 0.0f, 40.0f);
	if (type == LightType::Spot)
	{
		ImGui::SliderAngle("Angle", &angle, 0.0f, 180.0f);
	}
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
			light.angle = this->angle;
		}
	};

}

void LightComponent::destroy()
{
	g_engine.reg->remove<core_light_component>(this->entity->internal_entity);
}
