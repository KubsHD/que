#pragma once

#include <core/types.h>
#include <vector>
#include <entt/entt.hpp>
#include <SDL3/SDL.h>

class Renderer2;

class EditorPlatform
{
public:
	EditorPlatform();
	~EditorPlatform();

	void init(entt::registry& reg);
	void destroy();

	bool poll();
	void render();

	std::vector<String> get_requested_extensions();

	Renderer2* get_renderer() {
		return m_renderer;
	}

private:
	Renderer2* m_renderer;
	SDL_Window* m_internal_window;
	SDL_Surface m_surface;

};