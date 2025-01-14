#pragma once

#include <core/types.h>

#include <vulkan/vulkan.h>

namespace vkutil
{
	void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout, int mipLevel = -1, bool depth = false);
	void generate_mipmaps(VkCommandBuffer cmd, VkImage image, VkExtent2D size);
	void copy_image_to_image(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D src_size, VkExtent2D dst_size);

} // namespace vkinit
