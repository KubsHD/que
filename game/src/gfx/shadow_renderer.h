#pragma once

#include <core/types.h>

#include <gfx/rhi/gfx_device.h>

class Renderer2;

struct ShadowRenderer {

	void create(Renderer2& ren);

	void render(VkCommandBuffer cmd, entt::registry& reg);

	void destroy();



	GPUImage shadow_map;
	GPUPipeline dir_light_sm_pipeline;
};