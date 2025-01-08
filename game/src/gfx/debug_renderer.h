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

struct DebugRenderer {
	void init(Renderer2* r2);
	void destroy();
	
	void begin_frame();
	void render(VkCommandBuffer cmd, VkRenderingInfo& info);
	void end_frame();

	void draw_box(Vec3 pos, Vec3 scale, Vec4 color = Colors::default_color);
	void draw_line(Vec3 start, Vec3 end, Vec4 color = Colors::default_color);
	void draw_sphere(Vec3 pos, float radius, Vec4 color = Colors::default_color);

private:

	void fill_buffer();

	Renderer2* m_r2 = nullptr;

	// im3d state
	GPUBuffer m_im3d_buffer;
	VkDescriptorSetLayout m_im3d_set_layout;
	VkDescriptorSet m_im3d_set;

	GPUPipeline m_points_pipeline;
	GPUPipeline m_lines_pipeline;
	GPUPipeline m_triangles_pipeline;
};