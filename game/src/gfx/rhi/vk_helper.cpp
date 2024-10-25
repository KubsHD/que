#include "pch.h"

#include "vk_helper.h"

namespace vkh {
    VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags)
    {
        VkFenceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;

        info.flags = flags;

        return info;
    }
    
    VkSemaphoreCreateInfo sem_create_info(VkSemaphoreCreateFlags flags)
    {
       VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;

        info.flags = flags;

        return info;
    }
}