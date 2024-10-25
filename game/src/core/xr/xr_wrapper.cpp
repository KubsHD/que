#include "pch.h"

#include "xr_wrapper.h"
#include <common/openxr_helper.h>

#include <iostream>
#include <common/DebugOutput.h>

XrInstance Xr::instance;
XrSystemId Xr::systemId;


PFN_xrGetVulkanGraphicsDeviceKHR Xr::xrGetVulkanGraphicsDeviceKHR;
PFN_xrGetVulkanGraphicsRequirementsKHR Xr::xrGetVulkanGraphicsRequirementsKHR;
PFN_xrGetVulkanInstanceExtensionsKHR Xr::xrGetVulkanInstanceExtensionsKHR;
PFN_xrGetVulkanDeviceExtensionsKHR Xr::xrGetVulkanDeviceExtensionsKHR;

static const std::vector<char*> layerNames;
static const char* const extensionNames[] = {
	"XR_KHR_vulkan_enable",
	"XR_KHR_vulkan_enable2",
	"XR_EXT_debug_utils"
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
	xrGetVulkanGraphicsDeviceKHR(instance, systemId, vkInstance, &physicalDevice);

	return physicalDevice;
}

void Xr::GetVulkanGraphicsRequirements()
{
	XrGraphicsRequirementsVulkanKHR graphicsRequirements{};
	graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;

	xrGetVulkanGraphicsRequirementsKHR(instance, systemId, &graphicsRequirements);
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
	OPENXR_CHECK_PORTABLE(instance, xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsRequirementsKHR), "Failed to get InstanceProcAddr for xrGetVulkanGraphicsRequirementsKHR.");
	
	OPENXR_CHECK_PORTABLE(instance, xrGetInstanceProcAddr(instance, "xrGetVulkanDeviceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanDeviceExtensionsKHR), "Failed to get InstanceProcAddr for xrGetVulkanDeviceExtensionsKHR.");

	OPENXR_CHECK_PORTABLE(instance, xrGetInstanceProcAddr(instance, "xrGetVulkanInstanceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanInstanceExtensionsKHR), "Failed to get InstanceProcAddr for xrGetVulkanInstanceExtensionsKHR.");

	OPENXR_CHECK_PORTABLE(instance, xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsDeviceKHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsDeviceKHR), "Failed to get InstanceProcAddr for xrGetVulkanGraphicsDeviceKHR.");
}
