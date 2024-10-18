#include "pch.h"

#include "gfx_swapchain.h"
#include "gfx_device.h"
#include <SDL3/SDL_vulkan.h>
#include <lib/VkBootstrap.h>


GfxSwapchain::GfxSwapchain(SDL_Window* window)
{
	SDL_Vulkan_CreateSurface(window, GfxDevice::instance, nullptr, &m_surface);

	int width, height;
	SDL_GetWindowSizeInPixels(window, &width, &height);

	vkb::SwapchainBuilder swapchainBuilder{ GfxDevice::physical_device, GfxDevice::device, m_surface };

	m_swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		//.use_default_format_selection()
		.set_desired_format(VkSurfaceFormatKHR{ m_swapchainImageFormat, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build()
		.value();

	m_swapchainExtent = vkbSwapchain.extent;
	m_swapchain = vkbSwapchain.swapchain;
	
	m_swapchainImages = vkbSwapchain.get_images().value();
	m_swapchainImageViews = vkbSwapchain.get_image_views().value();
}

GfxSwapchain::~GfxSwapchain()
{
	vkDestroySwapchainKHR(GfxDevice::device, m_swapchain, nullptr);

	// destroy swapchain resources
	for (int i = 0; i < m_swapchainImageViews.size(); i++) {

		vkDestroyImageView(GfxDevice::device, m_swapchainImageViews[i], nullptr);
	}
}

void GfxSwapchain::Clear()
{
}

void GfxSwapchain::Present()
{
}
