#include "pch.h"

#include "renderer2.h"
#include "rhi/vk_helper.h"
#include "rhi/gfx_device.h"
#include "rhi/gfx_swapchain.h"

Renderer2::Renderer2()
{
    m_queue = GfxDevice::get_queue(vkb::QueueType::graphics);
	m_queue_family = GfxDevice::get_queue_family(vkb::QueueType::graphics);

    VkFenceCreateInfo finfo = vkh::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo seminfo = vkh::sem_create_info();

	VULKAN_CHECK_NOMSG(vkCreateFence(GfxDevice::device, &finfo, nullptr, &frame.main_fence));
	VULKAN_CHECK_NOMSG(vkCreateSemaphore(GfxDevice::device, &seminfo, nullptr, &frame.swapchain_semaphore));
	VULKAN_CHECK_NOMSG(vkCreateSemaphore(GfxDevice::device, &seminfo, nullptr, &frame.render_semaphore));

    
}

void Renderer2::draw()
{
	// clear 



	// present
}
