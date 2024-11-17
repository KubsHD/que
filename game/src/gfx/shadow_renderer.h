#pragma once

#include <core/types.h>

#include <gfx/rhi/gfx_device.h>

class Renderer2;

struct ShadowRenderer {

	void create(Renderer2& ren);

	void render(VkCommandBuffer cmd, entt::registry& reg);

	void destroy();

	glm::mat4 light_mtx;


	GPUImage shadow_map;
	VkSampler shadow_map_sampler;

	GPUPipeline dir_light_sm_pipeline;
};