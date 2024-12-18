#include "pch.h"

#include "xr_wrapper.h"
#include <common/openxr_helper.h>

#include <iostream>
#include <common/DebugOutput.h>

XrInstance Xr::instance;
XrSystemId Xr::systemId;


PFN_xrGetVulkanGraphicsDevice2KHR Xr::xrGetVulkanGraphicsDevice2KHR;
PFN_xrGetVulkanGraphicsRequirements2KHR Xr::xrGetVulkanGraphicsRequirements2KHR;
PFN_xrGetVulkanInstanceExtensionsKHR Xr::xrGetVulkanInstanceExtensionsKHR;
PFN_xrGetVulkanDeviceExtensionsKHR Xr::xrGetVulkanDeviceExtensionsKHR;

void Xr::Destroy()
{
	xrDestroyInstance(instance);
}

static const std::vector<char*> layerNames;
static const char* const extensionNames[] = {
	XR_KHR_VULKAN_ENABLE_EXTENSION_NAME,
	XR_EXT_DEBUG_UTILS_EXTENSION_NAME,
	XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME
};


void Xr::Init()
{

	XrApplicationInfo appInfo{};
	appInfo.apiVersion = XR_CURRENT_API_VERSION;
	strcpy(appInfo.applicationName, "Que");
	appInfo.applicationVersion = 1;
	strcpy(appInfo.engineName, "No Engine");
	appInfo.engineVersion = 1;


	uint32_t extensionCount = 0;
	std::vector<XrExtensionProperties> extensionProperties;
	OPENXR_CHECK_PORTABLE(instance, xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr), "Failed to enumerate InstanceExtensionProperties.");
	extensionProperties.resize(extensionCount, { XR_TYPE_EXTENSION_PROPERTIES });
	OPENXR_CHECK_PORTABLE(instance, xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data()), "Failed to enumerate InstanceExtensionProperties.");

	XrInstanceCreateInfo createInfo{};
	createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
	createInfo.createFlags = 0;
	createInfo.applicationInfo = appInfo;
	createInfo.enabledApiLayerCount = layerNames.size();
	createInfo.enabledApiLayerNames = layerNames.data();
	createInfo.enabledExtensionCount = sizeof(extensionNames) / sizeof(const char*);
	createInfo.enabledExtensionNames = extensionNames;
	OPENXR_CHECK_PORTABLE(instance, xrCreateInstance(&createInfo, &instance), "Failed to create Instance.");



	// Get the XrSystemId from the instance and the supplied XrFormFactor.
	XrSystemProperties m_xrSystemProperties = { XR_TYPE_SYSTEM_PROPERTIES };
	XrSystemGetInfo systemGI{ XR_TYPE_SYSTEM_GET_INFO };

	systemGI.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	OPENXR_CHECK_PORTABLE(Xr::instance, xrGetSystem(Xr::instance, &systemGI, &Xr::systemId), "Failed to get SystemID.");

	// Get the System's properties for some general information about the hardware and the vendor.
	OPENXR_CHECK_PORTABLE(Xr::instance, xrGetSystemProperties(Xr::instance, Xr::systemId, &m_xrSystemProperties), "Failed to get SystemProperties.");

	LoadPFN_XrFunctions();
}

VkPhysicalDevice Xr::GetVulkanGraphicsDevice(VkInstance vkInstance)
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	XrVulkanGraphicsDeviceGetInfoKHR deviceInfo{};
	deviceInfo.type = XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR;
	deviceInfo.next = nullptr;
	deviceInfo.systemId = systemId;
	deviceInfo.vulkanInstance = vkInstance;


	OPENXR_CHECK_PORTABLE(Xr::instance, xrGetVulkanGraphicsDevice2KHR(Xr::instance, &deviceInfo, &physicalDevice), "Failed to get physical device");

	return physicalDevice;
}

void Xr::GetVulkanGraphicsRequirements()
{
	XrGraphicsRequirementsVulkan2KHR graphicsRequirements{};
	graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;

	OPENXR_CHECK_PORTABLE(Xr::instance, xrGetVulkanGraphicsRequirements2KHR(instance, systemId, &graphicsRequirements), "Failed to get gfx req");

	LOG_INFO("min api version: " << XR_VERSION_MAJOR(graphicsRequirements.minApiVersionSupported) << "." << XR_VERSION_MINOR(graphicsRequirements.minApiVersionSupported));
	LOG_INFO("max api version: " << XR_VERSION_MAJOR(graphicsRequirements.maxApiVersionSupported) << "." << XR_VERSION_MINOR(graphicsRequirements.maxApiVersionSupported));

}

const std::vector<std::string> Xr::GetVulkanInstanceExtensions()
{
	uint32_t m_vulkanInstanceExtensionCount = 0;
	xrGetVulkanInstanceExtensionsKHR(instance, systemId, 0, &m_vulkanInstanceExtensionCount, nullptr);

	std::vector<char> m_vulkanInstanceExtensions(m_vulkanInstanceExtensionCount);
	xrGetVulkanInstanceExtensionsKHR(instance, systemId, m_vulkanInstanceExtensionCount, &m_vulkanInstanceExtensionCount, m_vulkanInstanceExtensions.data());

	std::stringstream streamData(m_vulkanInstanceExtensions.data());
	std::vector<std::string> extensions;
	std::string extension;
	while (std::getline(streamData, extension, ' ')) {
		extensions.push_back(extension);
	}
	return extensions;
}

void Xr::LoadPFN_XrFunctions()
{
	OPENXR_CHECK_PORTABLE(instance, xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsRequirements2KHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsRequirements2KHR), "Failed to get InstanceProcAddr for xrGetVulkanGraphicsRequirementsKHR.");
	
	OPENXR_CHECK_PORTABLE(instance, xrGetInstanceProcAddr(instance, "xrGetVulkanDeviceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanDeviceExtensionsKHR), "Failed to get InstanceProcAddr for xrGetVulkanDeviceExtensionsKHR.");

	OPENXR_CHECK_PORTABLE(instance, xrGetInstanceProcAddr(instance, "xrGetVulkanInstanceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanInstanceExtensionsKHR), "Failed to get InstanceProcAddr for xrGetVulkanInstanceExtensionsKHR.");

	OPENXR_CHECK_PORTABLE(instance, xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsDevice2KHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsDevice2KHR), "Failed to get InstanceProcAddr for xrGetVulkanGraphicsDeviceKHR.");
}
