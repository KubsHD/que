#pragma once

#include <core/types.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

class Renderer2;

namespace Colors
{
	const Vec4 default_color = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
}

struct DebugRenderer {
	void init(Renderer2* r2);
	
	void render(VkCommandBuffer cmd);

	void draw_box(Vec3 pos, Vec3 scale, Vec4 color = Colors::default_color);
	void draw_line(Vec3 start, Vec3 end, Vec4 color = Colors::default_color);
	void draw_sphere(Vec3 pos, float radius, Vec4 color = Colors::default_color);
};