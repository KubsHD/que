#include "pch.h"
#include "editor_platform.h"

#include <SDL3/SDL.h>
#include <gfx/rhi/gfx_device.h>
#include <gfx/renderer2.h>
#include <SDL3/SDL_vulkan.h>
#include <lib/imgui/imgui_impl_vulkan.h>

VkSurfaceKHR surf;

static ImGui_ImplVulkanH_Window g_MainWindowData;


EditorPlatform::EditorPlatform()
{
	SDL_Init(SDL_INIT_VIDEO);
}

EditorPlatform::~EditorPlatform()
{

}

void EditorPlatform::init(entt::registry& reg)
{

	GfxDevice::Init({"VK_KHR_surface"});

	m_internal_window = SDL_CreateWindow("QuEdit", 1280, 720, SDL_WINDOW_VULKAN);

	SDL_Vulkan_CreateSurface(m_internal_window, GfxDevice::instance, nullptr, &surf);

	int width, height;
	SDL_GetWindowSizeInPixels(m_internal_window, &width, &height);

	vkb::SwapchainBuilder swapchainBuilder{ GfxDevice::physical_device, GfxDevice::device, surf };

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		//.use_default_format_selection()
		.set_desired_format(VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build()
		.value();

	Swapchain sw;

	sw.swapchainVk = vkbSwapchain.swapchain;

	sw.swapchainImagesVk = vkbSwapchain.get_images().value();
	sw.swapchainImages = vkbSwapchain.get_image_views().value();

	sw.width = vkbSwapchain.extent.width;
	sw.height = vkbSwapchain.extent.height;

	sw.swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;

	// init renderer
	m_renderer = new Renderer2(sw, reg);


	// init imgui

	ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
}

void EditorPlatform::destroy()
{
	delete m_renderer;
	SDL_DestroyWindow(m_internal_window);
	SDL_Quit();
}

bool EditorPlatform::poll()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			return false;
			break;
		}
	}

	return true;
}

void EditorPlatform::render()
{
}

std::vector<String> EditorPlatform::get_requested_extensions()
{
	return std::vector<String>();
}
