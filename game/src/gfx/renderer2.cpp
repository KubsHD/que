#include "pch.h"

#include "renderer2.h"
#include "rhi/vk_helper.h"
#include "rhi/gfx_device.h"
#include "rhi/gfx_swapchain.h"
#include <common/vk_initializers.h>
#include <common/vk_image.h>

Renderer2::Renderer2()
{
    m_queue = GfxDevice::get_queue(vkb::QueueType::graphics);
	m_queue_family = GfxDevice::get_queue_family(vkb::QueueType::graphics);

    VkFenceCreateInfo finfo = vkh::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo seminfo = vkh::sem_create_info();

	VULKAN_CHECK_NOMSG(vkCreateFence(GfxDevice::device, &finfo, nullptr, &frame.main_fence));
	VULKAN_CHECK_NOMSG(vkCreateSemaphore(GfxDevice::device, &seminfo, nullptr, &frame.swapchain_semaphore));
	VULKAN_CHECK_NOMSG(vkCreateSemaphore(GfxDevice::device, &seminfo, nullptr, &frame.render_semaphore));

	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = m_queue_family;
	VULKAN_CHECK_NOMSG(vkCreateCommandPool(GfxDevice::device, &commandPoolInfo, nullptr, &frame.command_pool));

	VkCommandBufferAllocateInfo cmdAllocInfo = {};
	cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocInfo.pNext = nullptr;
	cmdAllocInfo.commandPool = frame.command_pool;
	cmdAllocInfo.commandBufferCount = 1;
	cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VULKAN_CHECK_NOMSG(vkAllocateCommandBuffers(GfxDevice::device, &cmdAllocInfo, &frame.main_command_buffer));
}

Renderer2::~Renderer2()
{
	vkDestroyFence(GfxDevice::device, frame.main_fence, nullptr);
	vkDestroySemaphore(GfxDevice::device, frame.swapchain_semaphore, nullptr);
	vkDestroySemaphore(GfxDevice::device, frame.render_semaphore, nullptr);
	vkDestroyCommandPool(GfxDevice::device, frame.command_pool, nullptr);
}

int _frameNumber = 0;

void Renderer2::draw(VkImage swapchain_image)
{
	VULKAN_CHECK_NOMSG(vkWaitForFences(GfxDevice::device, 1, &frame.main_fence, true, UINT64_MAX), "Failed to wait for Fence");
	VULKAN_CHECK_NOMSG(vkResetFences(GfxDevice::device, 1, &frame.main_fence), "Failed to reset Fence.")

	VkCommandBuffer cmd = frame.main_command_buffer;
	VULKAN_CHECK_NOMSG(vkResetCommandBuffer(cmd, 0));


	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	//start the command buffer recording
	VULKAN_CHECK_NOMSG(vkBeginCommandBuffer(cmd, &cmdBeginInfo));


	VkClearColorValue clearValue;
	float flash = std::abs(std::sin(_frameNumber / 120.f));
	clearValue = { { 0.0f, 0.0f, flash, 1.0f } };
	VkImageSubresourceRange clearRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

	vkinit::transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	vkCmdClearColorImage(cmd, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &clearRange);
	vkinit::transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


	VULKAN_CHECK_NOMSG(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdSubmitInfo = vkinit::command_buffer_submit_info(cmd);
	VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo2 submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

	VULKAN_CHECK_NOMSG(vkQueueSubmit2(m_queue, 1, &submitInfo, frame.main_fence));

	_frameNumber++;
}
