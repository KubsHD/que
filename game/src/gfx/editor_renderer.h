#pragma once

#include <core/types.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

class Renderer2;

struct EditorRenderer {
	void init(Renderer2* r2);
	
	void render(VkCommandBuffer cmd);
};