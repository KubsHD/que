#pragma once

#include <core/types.h>

#include <vulkan/vulkan.h>

namespace vkinit
{
	void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

} // namespace vkinit
