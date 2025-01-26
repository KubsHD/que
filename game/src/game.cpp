#include "pch.h"

#include "game.h"
#include <core/profiler.h>
#include <gfx/rhi/gfx_device.h>

#include <game/components/player_component.h>
#include <game/scenes/game_scene.h>

#include <asset/asset_manager.h>
#include <entt/entt.hpp>

#include <core/editor/editor_platform.h>
#include <game/scenes/dev_scene.h>

#include <gfx/debug_renderer.h>

template<typename T>
void Game::change_scene()
{
	if (m_current_scene != NULL)
	{
		m_current_scene->destroy();
		delete m_current_scene;
	}

	m_current_scene = new T();

	m_current_scene->engine.asset = new AssetManager();
	m_current_scene->engine.audio = m_audio_system.get();
	m_current_scene->engine.physics = m_physics_system.get();
	m_current_scene->engine.reg = &m_registry;
	m_current_scene->engine.render = platform->get_renderer();
	m_current_scene->engine.input = platform->input.get();

	m_current_scene->init();

	m_current_player_component = m_current_scene->get_first_component_of_type<PlayerComponent>();
}


bool g_game_running = true;
EditorPlatform* ed_platform;

Game::Game()
{
	QUE_PROFILE;

	m_audio_system = std::make_unique<AudioSystem>();


	platform = new OpenXRPlatform();
	platform->init(m_registry);

	AssetManager::Init(*m_audio_system, *platform->get_renderer());
	PhysicsSystem::init_static();
	m_physics_system = std::make_unique<PhysicsSystem>();

	platform->get_renderer()->load_default_resources();

	init_imgui();

	g_engine.audio = m_audio_system.get();
	g_engine.physics = m_physics_system.get();
	g_engine.reg = &m_registry;
	g_engine.input = platform->input.get();
	g_engine.asset = new AssetManager();
	g_engine.render = platform->get_renderer();

	change_scene<DevScene>();
}

Game::~Game()
{
	AssetManager::Destroy();
	platform->destroy();
	delete platform;
}

void Game::run()
{
	while (platform->is_running) {
		if (g_game_running)
		{
			platform->poll();

			ImGui::NewFrame();
			ImGui::ShowDemoWindow();

			g_engine.render->debug->begin_frame();

			update();

	

			Vec3 pos = Vec3(0, 0, 0);
			Quat rot = Quat(0, 0, 0, 1);
			Vec3 scale = Vec3(1, 1, 1);
			Im3d::Gizmo("Gizmo", &pos[0], &rot[0], &scale[0]);


			g_engine.render->debug->end_frame();
			ImGui::EndFrame();

			ImGui::Render();
			platform->render();
		}
	}

}

void Game::update()
{
	QUE_PROFILE;

	platform->update();

	m_physics_system->update(1.0f / 60.0f, m_registry);

	if (m_current_scene)
		m_current_scene->update();

	if (m_current_player_component)
		m_audio_system->update(m_current_player_component->entity->position);
	else
		m_audio_system->update(glm::vec3(0, 0, 0));

	if (m_current_scene)
		m_current_scene->draw_imgui();
}

void Game::init_imgui()
{
	NetImgui::Startup();
	NetImgui::ConnectFromApp("que", 8810);
}
