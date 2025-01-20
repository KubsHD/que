#include "pch.h"

#include "gfx_device.h"

#include <lib/vk_mem_alloc.h>

#include <common/DebugOutput.h>
#include <common/openxr_helper.h>
#include <core/xr/xr_wrapper.h>
#include <common/vk_initializers.h>
#include "vk_image.h"

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

	//	 ignore unused vertex shader input warning		steamvr ignorable validation error					ngfx-only errors
	if (pCallbackData->messageIdNumber == 101294395 || pCallbackData->messageIdNumber == 1303270965 || pCallbackData->messageIdNumber == 657209523)
		return VK_FALSE;

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
	volkInitialize();

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
		.request_validation_layers()
#if _DEBUG
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
	
	volkLoadInstance(inst_ret.value());

	vkb_internal::instance = inst_ret.value();
	instance = inst_ret.value().instance;

	VkPhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;

	VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features12.scalarBlockLayout = true;

	VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features13.dynamicRendering = true;
	features13.synchronization2 = true;

	// physical device
	vkb::PhysicalDeviceSelector selector{ vkb_internal::instance };
	auto phys_ret = selector/*.set_surface(surface)*/
		.defer_surface_initialization()
		.defer_surface_initialization()
		.set_required_features(features)
		.set_required_features_12(features12)
		.set_required_features_13(features13)
		.add_desired_extension(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME)
		.set_minimum_version(1, 3)
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

	InitCommon();
}

void GfxDevice::InitXr(XrInstance xri, XrSystemId xrsi)
{
	volkInitialize();

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
		.request_validation_layers()
		.set_debug_callback(debugCallback)
		.require_api_version(1, 3);
#if _DEBUG
#endif

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

	// print vulkan version
	uint32_t apiVersion = 0;
	vkEnumerateInstanceVersion(&apiVersion);
	LOG_INFO("Vulkan API Version: " << VK_VERSION_MAJOR(apiVersion) << "." << VK_VERSION_MINOR(apiVersion) << "." << VK_VERSION_PATCH(apiVersion));

	volkLoadInstance(inst_ret.value());

	vkb_internal::instance = inst_ret.value();
	instance = inst_ret.value().instance;

	VkPhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;
	features.geometryShader = true;

	VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features13.dynamicRendering = true;
	features13.synchronization2 = true;

	// physical device
	vkb::PhysicalDeviceSelector selector{ vkb_internal::instance };
	auto phys_ret = selector/*.set_surface(surface)*/
		.defer_surface_initialization()
		.set_required_features(features)
		.set_required_features_13(features13)
		.set_minimum_version(1, 3)

		// THIS FIXES CRASH UNDER STEAMVR
		.add_desired_extension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME)
		// -----------------------------------
		.add_desired_extension(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME)

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

	VmaVulkanFunctions vma_vulkan_func{};
	vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
	vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
	vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
	vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
	vma_vulkan_func.vkCreateImage = vkCreateImage;
	vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
	vma_vulkan_func.vkDestroyImage = vkDestroyImage;
	vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vma_vulkan_func.vkFreeMemory = vkFreeMemory;
	vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vma_vulkan_func.vkMapMemory = vkMapMemory;
	vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
	vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;

	allocatorCI.pVulkanFunctions = &vma_vulkan_func;

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

	LoadPFN_VkFunctions(instance);
}


void GfxDevice::Destroy()
{
	vkDestroyCommandPool(device, m_upload_context.pool, nullptr);
	vkDestroyFence(device, m_upload_context.uploadFence, nullptr);

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

GPUImage GfxDevice::create_image(VkExtent2D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
{
	GPUImage img{};

	img.format = format;
	img.size = size;

	VkImageCreateInfo iinfo = vkinit::image_create_info(format, usage, { size.width, size.height, 1 });

	VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D16_UNORM) {
		aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	if (mipmapped) {
		iinfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
	}

	VmaAllocationCreateInfo alloc_info{};
	alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


	vmaCreateImage(allocator, &iinfo, &alloc_info, &img.image, &img.allocation, &img.allocation_info);

	VkImageViewCreateInfo view_info = vkinit::imageview_create_info(format, img.image, aspectFlag);
	view_info.subresourceRange.levelCount = iinfo.mipLevels;

	VULKAN_CHECK_NOMSG(vkCreateImageView(device, &view_info, nullptr, &img.view));

	return img;
}

GPUImage GfxDevice::create_image(void* data, VkExtent2D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
{
	GPUImage img = create_image(size, format, usage, mipmapped);

	int pixels = img.size.width * img.size.height * 4;

	if (format == VK_FORMAT_R32G32B32A32_SFLOAT)
		pixels *= sizeof(float);

	upload_image(img, data, pixels, mipmapped);


	return img;
}

void GfxDevice::upload_image(GPUImage image, void* data, int size, bool mipmapped, int layer, int mipLevel)
{
	GPUBuffer staging = create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	upload_buffer(staging, 0, data, size);

	immediate_submit([&](VkCommandBuffer cmd) {

		VkBufferImageCopy copy{};

		VkExtent2D size = image.size;

		for (int i = 0; i < mipLevel; i++)
		{
			size.width /= 2;
			size.height /= 2;
		}

		copy.imageExtent = { size.width, size.height, 1 };
		
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.layerCount = 1;
		copy.imageSubresource.mipLevel = mipLevel;
		copy.imageSubresource.baseArrayLayer = layer;

		copy.bufferOffset = 0;
		copy.bufferRowLength = 0;
		copy.bufferImageHeight = 0;

		vkutil::transition_image(cmd, image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		vkCmdCopyBufferToImage(cmd, staging.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

		if (mipmapped)
			vkutil::generate_mipmaps(cmd, image.image, image.size);
		else
			vkutil::transition_image(cmd, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});

	destroy_buffer(staging);
}

void GfxDevice::destroy_image(GPUImage image)
{
	vmaDestroyImage(allocator, image.image, image.allocation);
	vkDestroyImageView(device, image.view, nullptr);
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

void GfxDevice::upload_buffer(GPUBuffer buffer, size_t offset, void* data, int size)
{
	void* mappedData = nullptr;
	VULKAN_CHECK(vmaMapMemory(allocator, buffer.allocation, &mappedData), "Failed to map memory!");
	if (mappedData && data) {
		memcpy((char*)mappedData + offset, data, size);
	}

	vmaUnmapMemory(allocator, buffer.allocation);
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

void GfxDevice::set_debug_name(VkDescriptorSet object, const std::string& name)
{
#ifndef _DEBUG
	return;
#endif

	VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	name_info.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
	name_info.objectHandle = (uint64_t)object;
	name_info.pObjectName = name.c_str();
	vkSetDebugUtilsObjectNameEXT(device, &name_info);
}
