#include "pch.h"

#include "gfx_device.h"

#include <lib/vk_mem_alloc.h>

#include <common/DebugOutput.h>
#include <common/openxr_helper.h>


VkInstance GfxDevice::instance;
VkDevice GfxDevice::device;
VkQueue GfxDevice::graphics_queue;
VkPhysicalDevice GfxDevice::physical_device;

PFN_vkSetDebugUtilsObjectNameEXT GfxDevice::vkSetDebugUtilsObjectNameEXT;
PFN_vkCreateDebugUtilsMessengerEXT GfxDevice::vkCreateDebugUtilsMessengerEXT;


#define VULKAN_CHECK(x, y)                                                                         \
    {                                                                                              \
        VkResult result = (x);                                                                     \
        if (result != VK_SUCCESS) {                                                                \
            std::cerr << "ERROR: VULKAN: " << std::hex << "0x" << result << std::dec << std::endl; \
            std::cerr << "ERROR: VULKAN: "  << string_VkResult(result) << std::endl; \
            std::cerr << "ERROR: VULKAN: " << y << std::endl;                                      \
        }                                                                                          \
    }

#define VULKAN_CHECK_NOMSG(x)                                                                         \
    {                                                                                              \
        VkResult result = (x);                                                                     \
        if (result != VK_SUCCESS) {                                                                \
            std::cerr << "ERROR: VULKAN: " << std::hex << "0x" << result << std::dec << std::endl; \
            std::cerr << "ERROR: VULKAN: "  << string_VkResult(result) << std::endl; \
        }                                                                                          \
    }

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "ERROR: VALIDATION: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void GfxDevice::LoadPFN_VkFunctions(VkInstance instance)
{
	GfxDevice::vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
	GfxDevice::vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

}

namespace vkb_internal {
	vkb::Instance instance;
	vkb::Device device;
	vkb::PhysicalDevice physical_device;
}

namespace gpu {
	VmaAllocator allocator;
}

void GfxDevice::Init(const std::vector<std::string>& requested_extensions)
{
	auto system_info_ret = vkb::SystemInfo::get_system_info();
	if (!system_info_ret) {
		printf("%s\n", system_info_ret.error().message().c_str());
	}

	auto system_info = system_info_ret.value();

	LOG_INFO("Available Vulkan extensions:");

	for (auto ext : system_info.available_extensions) {
		LOG_INFO(ext.extensionName);
	}


	// instance
	vkb::InstanceBuilder builder;
	builder.set_app_name("Que")
#if _DEBUG
		.request_validation_layers()
		.set_debug_callback(debugCallback)
#endif
		.desire_api_version(1, 2);

	for (const auto& ext : requested_extensions)
	{
		builder.enable_extension(ext.c_str());
		LOG_INFO(ext);
	}

	auto inst_ret = builder.build();

	if (!inst_ret) {
		std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
		abort();
	}

	vkb_internal::instance = inst_ret.value();
	instance = inst_ret.value().instance;

	// physical device
	vkb::PhysicalDeviceSelector selector{ vkb_internal::instance };
	auto phys_ret = selector/*.set_surface(surface)*/
		.defer_surface_initialization()
		.set_minimum_version(1, 1) // require a vulkan 1.1 capable device
		.select();
	if (!phys_ret) {
		std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
		assert("Failed to select Vulkan Physical Device");
	}
	physical_device = phys_ret.value().physical_device;


	// device
	vkb::DeviceBuilder device_builder{ phys_ret.value() };
	auto dev_ret = device_builder.build();
	if (!dev_ret) {
		std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
		assert("Failed to create Vulkan device");
	}
	vkb_internal::device = dev_ret.value();
	device = dev_ret.value().device;

	// queue
	auto graphics_queue_ret = vkb_internal::device.get_queue(vkb::QueueType::graphics);
	if (!graphics_queue_ret) {
		std::cerr << "Failed to get graphics queue. Error: " << graphics_queue_ret.error().message() << "\n";
		assert("Failed to get graphics queue");
	}

	graphics_queue = graphics_queue_ret.value();

	// memory allocator 
	VmaAllocatorCreateInfo allocatorCI{};
	allocatorCI.physicalDevice = phys_ret.value();
	allocatorCI.device = device;
	allocatorCI.instance = instance;

	VULKAN_CHECK(vmaCreateAllocator(&allocatorCI, &gpu::allocator), "Failed to create VMA allocator.");


	
	
}
