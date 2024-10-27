#pragma once

// note: this is only used when running in non-vr mode since openxr provides its own swapchain

#include <vulkan/vulkan.h>
#include <SDL3/SDL_video.h>
#include <openxr/openxr.h>

class GfxSwapchain {
public:
	GfxSwapchain(
		SDL_Window* window
	);

	~GfxSwapchain();

	void Clear();

	void Present();

private:
	VkSurfaceKHR m_surface;
	VkSwapchainKHR m_swapchain;
	VkFormat m_swapchainImageFormat;
	VkExtent2D m_swapchainExtent;

	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
};

struct Swapchain {
	XrSwapchain swapchain = XR_NULL_HANDLE;
	VkFormat swapchainFormat = VK_FORMAT_MAX_ENUM;

	int width;
	int height;

	std::vector<VkImageView> swapchainImages;
	std::vector<XrSwapchainImageVulkanKHR> swapchainImageHandles;
};
