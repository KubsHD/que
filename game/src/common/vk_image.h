#pragma once

#include <core/types.h>

#include <vulkan/vulkan.h>

namespace vkinit
{
	void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout, int mipLevel = -1);
	void generate_mipmaps(VkCommandBuffer cmd, VkImage image, VkExtent2D size);

} // namespace vkinit
