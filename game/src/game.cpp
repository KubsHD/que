#include "pch.h"

#include "game.h"
#include <core/profiler.h>
#include <gfx/rhi/gfx_device.h>

#include <game/components/player_component.h>
#include <game/scenes/game_scene.h>

#include <asset/asset_manager.h>
#include <entt/entt.hpp>

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

Game::Game()
{
	m_audio_system = std::make_unique<AudioSystem>();

	AssetManager::Init(*m_audio_system.get());

	platform = new OpenXRPlatform();
	platform->init(m_registry);

	PhysicsSystem::init_static();
	m_physics_system = std::make_unique<PhysicsSystem>();


	init_imgui();

	g_engine.audio = m_audio_system.get();
	g_engine.physics = m_physics_system.get();
	g_engine.reg = &m_registry;
	g_engine.input = platform->input.get();

	// todo: to refactor all below

	g_engine.asset = new AssetManager();
	g_engine.render = platform->get_renderer();

	change_scene<GameScene>();
}

Game::~Game()
{
	platform->destroy();
	delete platform;
}

void Game::run()
{
	while (platform->is_running) {
		platform->poll();
		update();
		platform->render();
	}

}

void Game::update()
{
	QUE_PROFILE;

	m_physics_system->update(1.0f / 60.0f, m_registry);

	if (m_current_scene)
		m_current_scene->update();

	if (m_current_player_component)
		m_audio_system->update(m_current_player_component->entity->position);
	else
		m_audio_system->update(glm::vec3(0, 0, 0));
}

void Game::init_imgui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	io.Fonts->Build();
	io.Fonts->SetTexID(0);
	io.DisplaySize = ImVec2(8, 8);
	io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
	ImGui::StyleColorsDark();

	NetImgui::Startup();
	NetImgui::ConnectFromApp("que");
}
