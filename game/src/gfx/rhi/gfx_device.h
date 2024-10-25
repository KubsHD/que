#pragma once

#include <core/types.h>

#include <vulkan/vulkan.h>

#define NUM_FRAMES 3

class GfxDevice {
public:
	static void Init(const std::vector<std::string>& requested_extensions = {});
	static void InitXr(XrInstance xri, XrSystemId xrsi);

	static VkInstance instance;
	static VkDevice device;
	static VkQueue graphics_queue;
	static VkPhysicalDevice physical_device;

	static VkQueue get_queue(vkb::QueueType type);
	static uint32_t get_queue_family(vkb::QueueType type);

	// format helpers
	static uint32_t select_supported_color_format(const std::vector<int64_t>& formats);
	static uint32_t select_supported_depth_format(const std::vector<int64_t>& formats);

	static XrGraphicsBindingVulkanKHR create_xr_graphics_binding();

private:

	static void InitCommon();

	static void LoadPFN_VkFunctions(VkInstance instance);

	static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
};