#include "pch.h"
#include "editor_platform.h"

#include <SDL3/SDL.h>
#include <gfx/rhi/gfx_device.h>
#include <gfx/renderer2.h>
#include <gfx/imgui_renderer.h>
#include <SDL3/SDL_vulkan.h>
#include <lib/imgui/imgui_impl_vulkan.h>
#include <lib/imgui/imgui_impl_sdl3.h>


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

	vkb_swapchain = swapchainBuilder
		//.use_default_format_selection()
		.set_desired_format(VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
		.set_desired_extent(width, height)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build()
		.value();


	rt.image.image = vkb_swapchain.get_images().value()[0];
	rt.image.view = vkb_swapchain.get_image_views().value()[0];

	rt.size.x = vkb_swapchain.extent.width;
	rt.size.y = vkb_swapchain.extent.height;

	rt.format = vkb_swapchain.image_format;

	// init renderer
	m_renderer = new Renderer2(rt, reg);



	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	ImGui_ImplSDL3_InitForVulkan(m_internal_window);

	m_imgui_renderer = new ImguiRenderer();
	m_imgui_renderer->init(m_renderer);
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

void EditorPlatform::render(Camera cam)
{

	CameraRenderData crd;

	crd.position = cam.position;
	crd.projection = glm::perspective(glm::radians(90.0f), 16.0f / 9.0f, 0.1f, 1000.0f);
	crd.view = glm::lookAt(cam.position, cam.position + cam.rotation * glm::vec3(0, 0, -1), cam.rotation * glm::vec3(0, 1, 0));



	m_renderer->wait_for_frame();
	rt.image = m_renderer->acquire_image(vkb_swapchain);

	m_renderer->draw(rt, crd);
	m_renderer->present(vkb_swapchain.swapchain);
}

std::vector<String> EditorPlatform::get_requested_extensions()
{
	return std::vector<String>();
}
