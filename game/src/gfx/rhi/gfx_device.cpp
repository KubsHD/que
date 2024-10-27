#include "pch.h"

#include "gfx_device.h"

#include <lib/vk_mem_alloc.h>

#include <common/DebugOutput.h>
#include <common/openxr_helper.h>
#include <core/xr/xr_wrapper.h>

namespace vkb_internal {
	vkb::Instance instance;
	vkb::Device device;
	vkb::PhysicalDevice physical_device;
}

namespace gpu {
}

VkInstance GfxDevice::instance;
VkDevice GfxDevice::device;
VkQueue GfxDevice::graphics_queue;
VkPhysicalDevice GfxDevice::physical_device;

VmaAllocator GfxDevice::allocator;

GfxDevice::UploadContext GfxDevice::m_upload_context;

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

const std::vector<int64_t> supported_color_formats{
		VK_FORMAT_B8G8R8A8_SRGB,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_FORMAT_R8G8B8A8_UNORM
};

const std::vector<int64_t> supported_depth_formats
	{
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM 
};



VkQueue GfxDevice::get_queue(vkb::QueueType type)
{
	return vkb_internal::device.get_queue(type).value();
}

uint32_t GfxDevice::get_queue_family(vkb::QueueType type)
{
	return vkb_internal::device.get_queue_index(type).value();
}

uint32_t GfxDevice::select_supported_color_format(const std::vector<int64_t>& formats)
{
	const std::vector<int64_t>::const_iterator& swapchainFormatIt = std::find_first_of(formats.begin(), formats.end(),
		std::begin(supported_color_formats), std::end(supported_color_formats));

	if (swapchainFormatIt == formats.end()) {
		std::cout << "ERROR: Unable to find supported Color Swapchain Format" << std::endl;
		DEBUG_BREAK;
		return 0;
	}

	return *swapchainFormatIt;
}

uint32_t GfxDevice::select_supported_depth_format(const std::vector<int64_t>& formats)
{
	const std::vector<int64_t>::const_iterator& swapchainFormatIt = std::find_first_of(formats.begin(), formats.end(),
		std::begin(supported_depth_formats), std::end(supported_depth_formats));
	if (swapchainFormatIt == formats.end()) {
		std::cout << "ERROR: Unable to find supported Depth Swapchain Format" << std::endl;
		DEBUG_BREAK;
		return 0;
	}
	return *swapchainFormatIt;
}

XrGraphicsBindingVulkanKHR GfxDevice::create_xr_graphics_binding()
{
	XrGraphicsBindingVulkanKHR binding{};

	binding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
	binding.instance = instance;
	binding.physicalDevice = physical_device;
	binding.device = device;
	binding.queueFamilyIndex = get_queue_family(vkb::QueueType::graphics);
	binding.queueIndex = vkb_internal::device.get_queue_index(vkb::QueueType::graphics).value();

	return binding;
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
		.desire_api_version(1, 3);

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
}

void GfxDevice::InitXr(XrInstance xri, XrSystemId xrsi)
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
		.desire_api_version(1, 3);

	auto exts = Xr::GetVulkanInstanceExtensions();

	for (const auto& ext : exts)
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

	VkPhysicalDeviceVulkan13Features features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features.dynamicRendering = true;
	features.synchronization2 = true;

	// physical device
	vkb::PhysicalDeviceSelector selector{ vkb_internal::instance };
	auto phys_ret = selector/*.set_surface(surface)*/
		.defer_surface_initialization()
		.set_required_features_13(features)
		.set_minimum_version(1, 3)
		.select();
	if (!phys_ret) {
		std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
		assert("Failed to select Vulkan Physical Device");
	}
	physical_device = phys_ret.value().physical_device;

	VkPhysicalDevice pd = Xr::GetVulkanGraphicsDevice(instance);

	assert(pd == physical_device);

	// device
	vkb::DeviceBuilder device_builder{ phys_ret.value() };
	

	auto dev_ret = device_builder.build();
	if (!dev_ret) {
		std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
		assert("Failed to create Vulkan device");
	}
	vkb_internal::device = dev_ret.value();
	device = dev_ret.value().device;

	InitCommon();
}


void GfxDevice::InitCommon()
{
	// queue
	auto graphics_queue_ret = vkb_internal::device.get_queue(vkb::QueueType::graphics);
	if (!graphics_queue_ret) {
		std::cerr << "Failed to get graphics queue. Error: " << graphics_queue_ret.error().message() << "\n";
		assert("Failed to get graphics queue");
	}

	graphics_queue = graphics_queue_ret.value();

	// memory allocator 
	VmaAllocatorCreateInfo allocatorCI{};
	allocatorCI.physicalDevice = physical_device;
	allocatorCI.device = device;
	allocatorCI.instance = instance;

	VULKAN_CHECK(vmaCreateAllocator(&allocatorCI, &allocator), "Failed to create VMA allocator.");

	// immiedate context

	VkFenceCreateInfo ufenceCI{};
	ufenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	ufenceCI.pNext = nullptr;
	VULKAN_CHECK(vkCreateFence(device, &ufenceCI, nullptr, &m_upload_context.uploadFence), "Failed to create Fence.");

	// craete upload commandpool
	VkCommandPoolCreateInfo uploadCommandPoolInfo{};
	uploadCommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	uploadCommandPoolInfo.pNext = nullptr;

	uploadCommandPoolInfo.flags = 0;
	uploadCommandPoolInfo.queueFamilyIndex = get_queue_family(vkb::QueueType::graphics);

	VULKAN_CHECK_NOMSG(vkCreateCommandPool(device, &uploadCommandPoolInfo, nullptr, &m_upload_context.pool));

	// create upload commandbuffer
	VkCommandBufferAllocateInfo allocateInfo2{};
	allocateInfo2.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo2.pNext = nullptr;
	allocateInfo2.commandPool = m_upload_context.pool;
	allocateInfo2.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo2.commandBufferCount = 1;
	VULKAN_CHECK(vkAllocateCommandBuffers(device, &allocateInfo2, &m_upload_context.buffer), "Failed to allocate CommandBuffers.");

}


void GfxDevice::Destroy()
{
	vmaDestroyAllocator(allocator);

	vkb::destroy_device(vkb_internal::device);
	vkb::destroy_instance(vkb_internal::instance);
}

void GfxDevice::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	VkCommandBuffer cmd = m_upload_context.buffer;

	VkCommandBufferBeginInfo cbbi{};
	cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.pNext = nullptr;

	cbbi.pInheritanceInfo = nullptr;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VULKAN_CHECK(vkBeginCommandBuffer(cmd, &cbbi), "Failed to begin cmd buf");

	function(cmd);

	VULKAN_CHECK(vkEndCommandBuffer(cmd), "Failed to end cmd buf");

	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = nullptr;

	info.waitSemaphoreCount = 0;
	info.pWaitSemaphores = nullptr;
	info.pWaitDstStageMask = nullptr;
	info.commandBufferCount = 1;
	info.pCommandBuffers = &cmd;
	info.signalSemaphoreCount = 0;
	info.pSignalSemaphores = nullptr;


	//submit command buffer to the queue and execute it.
	// _uploadFence will now block until the graphic commands finish execution
	VULKAN_CHECK(vkQueueSubmit(graphics_queue, 1, &info, m_upload_context.uploadFence), "Failed to submit buffer to queue");

	VULKAN_CHECK_NOMSG(vkWaitForFences(device, 1, &m_upload_context.uploadFence, true, 9999999999));
	VULKAN_CHECK_NOMSG(vkResetFences(device, 1, &m_upload_context.uploadFence));

	// reset the command buffers inside the command pool
	VULKAN_CHECK_NOMSG(vkResetCommandPool(device, m_upload_context.pool, 0));
}

VkImageView GfxDevice::create_image_view(VkImageViewCreateInfo ivinfo)
{
	VkImageView iv;
	VULKAN_CHECK(vkCreateImageView(device, &ivinfo, nullptr, &iv), "Failed to create image view.");
	return iv;
}

void GfxDevice::destroy_image_view(VkImageView imageView)
{
	vkDestroyImageView(device, imageView, nullptr);
}

VkPipeline GfxDevice::create_graphics_pipeline(VkGraphicsPipelineCreateInfo pipeline_info)
{
	return VkPipeline{};
}

void GfxDevice::destroy_pipeline(VkPipeline pipeline)
{

}

GPUBuffer GfxDevice::create_buffer(int size, VkBufferUsageFlags usage_flags, VmaMemoryUsage mem_usage)
{

	VkBufferCreateInfo buffer_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };

	buffer_info.size = size;
	buffer_info.usage = usage_flags;

	VmaAllocationCreateInfo  alloc_info{};
	alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	
	GPUBuffer buf;

	VULKAN_CHECK_NOMSG(vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &buf.buffer, &buf.allocation, &buf.allocation_info));

	return buf;
}

void GfxDevice::destroy_buffer(GPUBuffer buffer)
{
	vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
}

void GfxDevice::set_debug_name(VkBuffer object, const std::string& name)
{
#ifndef _DEBUG
	return;
#endif

	VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	name_info.objectType = VK_OBJECT_TYPE_BUFFER;
	name_info.objectHandle = (uint64_t)object;
	name_info.pObjectName = name.c_str();
	vkSetDebugUtilsObjectNameEXT(device, &name_info);
}

void GfxDevice::set_debug_name(VkImage object, const std::string& name)
{
#ifndef _DEBUG
	return;
#endif

	VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	name_info.objectType = VK_OBJECT_TYPE_IMAGE;
	name_info.objectHandle = (uint64_t)object;
	name_info.pObjectName = name.c_str();
	vkSetDebugUtilsObjectNameEXT(device, &name_info);
}

void GfxDevice::set_debug_name(VkImageView object, const std::string& name)
{
#ifndef _DEBUG
	return;
#endif

	VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	name_info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
	name_info.objectHandle = (uint64_t)object;
	name_info.pObjectName = name.c_str();
	vkSetDebugUtilsObjectNameEXT(device, &name_info);
}

void GfxDevice::set_debug_name(VkRenderPass object, const std::string& name)
{

}

void GfxDevice::set_debug_name(VkCommandBuffer object, const std::string& name)
{
#ifndef _DEBUG
	return;
#endif

	VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	name_info.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
	name_info.objectHandle = (uint64_t)object;
	name_info.pObjectName = name.c_str();
	vkSetDebugUtilsObjectNameEXT(device, &name_info);
}

void GfxDevice::set_debug_name(VkQueue object, const std::string& name)
{
#ifndef _DEBUG
	return;
#endif

	VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	name_info.objectType = VK_OBJECT_TYPE_QUEUE;
	name_info.objectHandle = (uint64_t)object;
	name_info.pObjectName = name.c_str();
	vkSetDebugUtilsObjectNameEXT(device, &name_info);
}
