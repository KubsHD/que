#pragma once

#include <core/types.h>

#include <gfx/rhi/gfx_device.h>

class Renderer2;

struct ShadowRenderer {

	void create(Renderer2& ren);

	void update();

	void render(VkCommandBuffer cmd, entt::registry& reg, glm::vec3 cam_pos);
	void render_imgui();

	void destroy();

	glm::mat4 light_mtx;

	GPUImage shadow_map;
	VkSampler shadow_map_sampler;
	GPUPipeline dir_light_sm_pipeline;

	float near_plane = 1.0f, far_plane = 50.0f, frustum_size = 20.0f, dl_height = 10.0f;
};