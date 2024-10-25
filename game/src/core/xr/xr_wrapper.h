#pragma once

#include <core/types.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <vulkan/vulkan.h>

class Xr {
public:
	static void Init();

	static XrInstance instance;
	static XrSystemId systemId;

	static VkPhysicalDevice GetVulkanGraphicsDevice(VkInstance vkInstance);
	static void GetVulkanGraphicsRequirements();

	static const std::vector<std::string> GetVulkanInstanceExtensions();

private:
	static void LoadPFN_XrFunctions();

	static PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR;
	static PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR;
	static PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR;
	static PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR;

};