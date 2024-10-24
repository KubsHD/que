#include "pch.h"

#include "renderer2.h"
#include "./rhi/vk_helper.h"

Renderer2::Renderer2()
{
    VkFenceCreateInfo finfo = vkh::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo seminfo = vkh::sem_create_info();

    for (int i = 0; i< NUM_FRAMES; i++)
    {
        VULKAN_CHECK(vkCreateFence(GfxDevice::device, &finfo, nullptr, &m_frames[i].main_fence));
        VULKAN_CHECK(vkCreateSemaphore(GfxDevice::device, &seminfo, nullptr, &m_frames[i].swapchain_semaphore));
        VULKAN_CHECK(vkCreateSemaphore(GfxDevice::device, &seminfo, nullptr, &m_frames[i].render_semaphore));
    }

    
}