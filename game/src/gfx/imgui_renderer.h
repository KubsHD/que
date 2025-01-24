#pragma once

#include <core/types.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "rhi/gfx_device.h"

class Renderer2;

namespace Colors
{
	const Vec4 default_color = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
}

struct ImguiRenderer {
	void init(Renderer2* r2);
	void destroy();

	void begin_frame();
	void render(VkCommandBuffer cmd);
	void end_frame();
private:
	GPUPipeline m_imgui_pipeline;
	VkDescriptorSetLayout m_imgui_set_layout;
	VkDescriptorSet m_imgui_set;
};

