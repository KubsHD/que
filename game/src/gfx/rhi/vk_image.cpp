#include "pch.h"

#include "vk_image.h"
#include <common/vk_initializers.h>

#include <cmath>

void vkutil::transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout, int mipLevel, bool depth)
{
	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	
	barrier.pNext = nullptr;
	barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.oldLayout = currentLayout;
	barrier.newLayout = newLayout;
	barrier.image = image;

	VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	if (depth)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	barrier.subresourceRange = vkinit::image_subresource_range(aspectMask);

	if (mipLevel > -1)
	{
		barrier.subresourceRange.baseMipLevel = mipLevel;
		barrier.subresourceRange.levelCount = 1;
	}

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void vkutil::generate_mipmaps(VkCommandBuffer cmd, VkImage image, VkExtent2D size)
{
	int mipLevels = int(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
	for (int mip = 0; mip < mipLevels; mip++) 
	{
		VkExtent2D halfSize = size;
		halfSize.width /= 2;
		halfSize.height /= 2;

		vkutil::transition_image(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mip);

		if (mip < mipLevels - 1) {
			VkImageBlit blitRegion{};

			blitRegion.srcOffsets[1].x = size.width;
			blitRegion.srcOffsets[1].y = size.height;
			blitRegion.srcOffsets[1].z = 1;

			blitRegion.dstOffsets[1].x = halfSize.width;
			blitRegion.dstOffsets[1].y = halfSize.height;
			blitRegion.dstOffsets[1].z = 1;

			blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegion.srcSubresource.baseArrayLayer = 0;
			blitRegion.srcSubresource.layerCount = 1;
			blitRegion.srcSubresource.mipLevel = mip;

			blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegion.dstSubresource.baseArrayLayer = 0;
			blitRegion.dstSubresource.layerCount = 1;
			blitRegion.dstSubresource.mipLevel = mip + 1;

			vkCmdBlitImage(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_LINEAR);

			size = halfSize;
		}
	}

	// transition all mip levels into the final read_only layout
	transition_image(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void vkutil::copy_image_to_image(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D src_size, VkExtent2D dst_size)
{
	VkImageBlit2 region{};
	region.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

	region.srcOffsets[1].x = src_size.width;
	region.srcOffsets[1].y = src_size.height;
	region.srcOffsets[1].z = 1;

	region.dstOffsets[1].x = dst_size.width;
	region.dstOffsets[1].y = dst_size.height;
	region.dstOffsets[1].z = 1;

	region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.srcSubresource.baseArrayLayer = 0;
	region.srcSubresource.layerCount = 1;
	region.srcSubresource.mipLevel = 0;

	region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.dstSubresource.baseArrayLayer = 0;
	region.dstSubresource.layerCount = 1;
	region.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blit_info{};

	blit_info.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	blit_info.srcImage = src;
	blit_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blit_info.dstImage = dst;
	blit_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blit_info.regionCount = 1;
	blit_info.pRegions = &region;
	blit_info.filter = VK_FILTER_LINEAR;

	vkCmdBlitImage2(cmd, &blit_info);
}
