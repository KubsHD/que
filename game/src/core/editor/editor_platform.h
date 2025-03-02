#pragma once

#include <core/types.h>
#include <vector>
#include <entt/entt.hpp>
#include <SDL3/SDL.h>
#include <gfx/rhi/gfx_swapchain.h>
#include <gfx/camera.h>
#include <gfx/renderer2.h>


class ImguiRenderer;

class EditorPlatform
{
public:
	EditorPlatform();
	~EditorPlatform();

	void init(entt::registry& reg);
	void destroy();

	void update();
	bool poll();
	void render(Camera cam);

	std::unordered_map<SDL_Scancode, bool> get_key_map();

	std::vector<String> get_requested_extensions();

	Renderer2* get_renderer() {
		return m_renderer;
	}



	SDL_Window* get_internal_window() {
		return m_internal_window;
	}
private:
	float speed = 0.01f;
	Renderer2* m_renderer;
	SDL_Window* m_internal_window;
	SDL_Surface m_surface;

	vkb::Swapchain vkb_swapchain;
	Vector<VkImage> swapchain_images;
	Vector<VkImageView> swapchain_image_views;
	int image_index;

	RenderTarget rt;

	std::chrono::high_resolution_clock::time_point m_last_time;
	float m_accumulator = 0.0f;


};