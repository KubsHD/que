#pragma once

#include <vulkan/vulkan.h>

#define VULKAN_CHECK(x, y)                                                                         \
    {                                                                                              \
        VkResult result = (x);                                                                     \
        if (result != VK_SUCCESS) {                                                                \
            std::cout << "ERROR: VULKAN: " << std::hex << "0x" << result << std::dec << std::endl; \
            std::cout << "ERROR: VULKAN: "  << string_VkResult(result) << std::endl; \
            std::cout << "ERROR: VULKAN: " << y << std::endl;                                      \
        }                                                                                          \
    }


#define VULKAN_CHECK_NOMSG(x)                                                                         \
    {                                                                                              \
        VkResult result = (x);                                                                     \
        if (result != VK_SUCCESS) {                                                                \
            std::cout << "ERROR: VULKAN: " << std::hex << "0x" << result << std::dec << std::endl; \
            std::cout << "ERROR: VULKAN: "  << string_VkResult(result) << std::endl; \
        }                                                                                          \
    }


namespace vkh {
    VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);
    VkSemaphoreCreateInfo sem_create_info(VkSemaphoreCreateFlags flags = 0);
}