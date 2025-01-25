#pragma once

#include <core/types.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "rhi/gfx_device.h"

class Renderer2;

struct ImguiRenderer {
	void init(Renderer2* r2);
	void destroy();

	void begin_frame();
	void render(VkCommandBuffer cmd);
	void end_frame();
private:
	void prepare_buffer(VkCommandBuffer cmd);

	Renderer2* m_r2 = nullptr;

	Vector<VkImageView> tex_views;

	GPUBuffer imgui_indices;
	GPUBuffer imgui_vertices;

	GPUImage imgui_font;

	GPUPipeline m_imgui_pipeline;
	VkDescriptorSetLayout m_imgui_set_layout;
	VkDescriptorSet m_imgui_set;
};

