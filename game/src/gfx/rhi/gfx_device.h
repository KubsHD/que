#pragma once

#include <core/types.h>

#include <vulkan/vulkan.h>

#define NUM_FRAMES 3

class GfxDevice {
public:
	static void Init(const std::vector<std::string>& requested_extensions = {});
	static void InitXr(XrInstance xri, XrSystemId xrsi);
	static void Destroy();

	static VkInstance instance;
	static VkDevice device;
	static VkQueue graphics_queue;
	static VkPhysicalDevice physical_device;

	static VkQueue get_queue(vkb::QueueType type);
	static uint32_t get_queue_family(vkb::QueueType type);

	// format helpers
	static uint32_t select_supported_color_format(const std::vector<int64_t>& formats);
	static uint32_t select_supported_depth_format(const std::vector<int64_t>& formats);

	// xr helpers
	static XrGraphicsBindingVulkanKHR create_xr_graphics_binding();

	// resources
	static VkImageView create_image_view(VkImageViewCreateInfo ivinfo);
	static void destroy_image_view(VkImageView imageView);

	//static VkImage create_image(VkImageCreateInfo iinfo);
	//static void destroy_image(VkImage image);

	//static VkBuffer create_buffer(VkBufferCreateInfo binfo);
	//static void destroy_buffer(VkBuffer buffer);

	// debug name helpers
	static void set_debug_name(VkBuffer object, const std::string& name);
	static void set_debug_name(VkImage object, const std::string& name);
	static void set_debug_name(VkImageView object, const std::string& name);
	static void set_debug_name(VkRenderPass object, const std::string& name);
	static void set_debug_name(VkCommandBuffer object, const std::string& name);
	static void set_debug_name(VkQueue object, const std::string& name);

private:
	static void InitCommon();

	static void LoadPFN_VkFunctions(VkInstance instance);

	static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
};