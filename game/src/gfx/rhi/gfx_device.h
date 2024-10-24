#pragma once

#include <core/types.h>

#include <vulkan/vulkan.h>

#define NUM_FRAMES 3

struct FrameData {
	VkSemaphore swapchain_semaphore;
	VkSemaphore render_semaphore;
	VkFence main_fence;
}

class GfxDevice {
public:
	static void Init(const std::vector<std::string>& requested_extensions = {});

	static VkInstance instance;
	static VkDevice device;
	static VkQueue graphics_queue;
	static VkPhysicalDevice physical_device;
private:

	static FrameData m_frames[NUM_FRAMES];

	static void LoadPFN_VkFunctions(VkInstance instance);

	static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
};