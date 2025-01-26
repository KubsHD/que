#include "pch.h"
#include "editor_platform.h"

#include <SDL3/SDL.h>
#include <gfx/rhi/gfx_device.h>
#include <gfx/renderer2.h>
#include <gfx/imgui_renderer.h>
#include <SDL3/SDL_vulkan.h>
#include <lib/imgui/imgui_impl_vulkan.h>
#include <lib/imgui/imgui_impl_sdl3.h>
#include <common/DebugOutput.h>


VkSurfaceKHR surf;

static ImGui_ImplVulkanH_Window g_MainWindowData;


static bool mouse_capture = false;
static Vec3 target_cam_pos = Vec3(0, 0, 0);

std::unordered_map<SDL_Scancode, bool> key_map;

EditorPlatform::EditorPlatform()
{
	SDL_Init(SDL_INIT_VIDEO);
}

EditorPlatform::~EditorPlatform()
{

}

const float FPS = 75.f;
const float dt = 1.f / FPS;

void EditorPlatform::init(entt::registry& reg)
{
	GfxDevice::Init({"VK_KHR_surface"});

	m_internal_window = SDL_CreateWindow("QuEdit", 1600, 900, SDL_WINDOW_VULKAN);

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

	swapchain_images = vkb_swapchain.get_images().value();
	swapchain_image_views = vkb_swapchain.get_image_views().value();

	rt.size.x = vkb_swapchain.extent.width;
	rt.size.y = vkb_swapchain.extent.height;

	rt.format = vkb_swapchain.image_format;

	// init renderer
	m_renderer = new Renderer2(rt, reg);

	ImGui_ImplSDL3_InitForVulkan(m_internal_window);
	image_index = 0;

	m_last_time = std::chrono::high_resolution_clock::now();
	m_accumulator = dt; // so that we get at least 1 update before render
}

static Camera test_cam;

void EditorPlatform::destroy()
{
	delete m_renderer;
	SDL_DestroyWindow(m_internal_window);
	SDL_Quit();
}

void EditorPlatform::update()
{

	if (mouse_capture)
	{
		if (key_map[SDL_SCANCODE_W])
		{
			target_cam_pos += test_cam.rotation * Vec3(0, 0, -speed);
		}

		if (key_map[SDL_SCANCODE_S])
		{
			target_cam_pos += test_cam.rotation * Vec3(0, 0, speed);
		}

		if (key_map[SDL_SCANCODE_A])
		{
			target_cam_pos += test_cam.rotation * Vec3(-speed, 0, 0);
		}

		if (key_map[SDL_SCANCODE_D])
		{
			target_cam_pos += test_cam.rotation * Vec3(speed, 0, 0);
		}

		if (key_map[SDL_SCANCODE_E])
		{
			target_cam_pos += Vec3(0, speed, 0);
		}

		if (key_map[SDL_SCANCODE_Q])
		{
			target_cam_pos += Vec3(0, -speed, 0);
		}
	}


	m_last_time = std::chrono::high_resolution_clock::now();

	// Delay to not overload the CPU
	const auto now = std::chrono::high_resolution_clock::now();
	const auto frameTime = std::chrono::duration<float>(now - m_last_time).count();
	if (dt > frameTime) {
		SDL_Delay(static_cast<std::uint32_t>(dt - frameTime));
	}
}


bool EditorPlatform::poll()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL3_ProcessEvent(&event);

		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			return false;
			break;
		case SDL_EVENT_KEY_DOWN:
			key_map[event.key.scancode] = true;

			break;
		case SDL_EVENT_KEY_UP:
			key_map[event.key.scancode] = false;
			break;
		case SDL_EVENT_MOUSE_MOTION:
			if (!mouse_capture)
				break;

			test_cam.rotation_euler.y -= (event.motion.xrel * 0.01f);
			test_cam.rotation_euler.x -= (event.motion.yrel * 0.01f);

			// cap rotation to 90 degrees
			if (test_cam.rotation_euler.x > 1.57f)
				test_cam.rotation_euler.x = 1.57f;

			if (test_cam.rotation_euler.x < -1.57f)
				test_cam.rotation_euler.x = -1.57f;
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				SDL_HideCursor();
				mouse_capture = true;
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				SDL_ShowCursor();
				mouse_capture = false;
			}
			break;
		}
	}

	return true;
}

void EditorPlatform::render(Camera cam)
{


	test_cam.rotation = glm::quat(test_cam.rotation_euler);

	CameraRenderData crd;

	test_cam.position = glm::lerp(test_cam.position, target_cam_pos, 0.01f);

	crd.position = test_cam.position;
	crd.projection = glm::perspective(glm::radians(90.0f), 16.0f / 9.0f, 0.1f, 1000.0f);

	crd.projection[1][1] *= -1;

	crd.view = glm::lookAt(test_cam.position, test_cam.position + test_cam.rotation * glm::vec3(0, 0, -1), test_cam.rotation * glm::vec3(0, 1, 0));


	m_renderer->wait_for_frame();
	image_index = m_renderer->acquire_image(vkb_swapchain);

	rt.image.image = swapchain_images[image_index];
	rt.image.view = swapchain_image_views[image_index];

	m_renderer->draw(rt, crd);
	m_renderer->present(vkb_swapchain.swapchain);
}

std::unordered_map<SDL_Scancode, bool> EditorPlatform::get_key_map()
{
	return key_map;
}

std::vector<String> EditorPlatform::get_requested_extensions()
{
	return std::vector<String>();
}
