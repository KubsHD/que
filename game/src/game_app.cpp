#include "pch.h"

#include "game_app.h"

#include <asset/mesh.h>

#include <core/asset.h>
#include <core/profiler.h>
#include <core/audio.h>

#include <gfx/renderer.h>
#include <gfx/pipeline/sky_pipeline.h>
#include <gfx/pipeline/mesh_pipeline.h>
#include <gfx/pipeline/sky_cube_render_pipeline.h>
#include <gfx/sky.h>

#include <common/GraphicsAPI.h>
#include <common/vk_initializers.h>
#include <common/glm_helpers.h>
#include <common/serialization.h>

#include <core/components/components.h>
#include <core/systems/render_system.h>
#include <core/physics_util.h>

#include <game/scenes/game_scene.h>
#include <game/components/player_component.h>

template<typename T>
void GameApp::change_scene()
{
	if (m_current_scene != NULL)
	{
		m_current_scene->destroy();
		delete m_current_scene;
	}

	m_current_scene = new T();

	m_current_scene->engine.asset = m_asset_manager.get();
	m_current_scene->engine.audio = m_audio_system.get();
	m_current_scene->engine.physics = m_physics_system.get();
	m_current_scene->engine.reg = &m_registry;
	m_current_scene->engine.render = m_renderer.get();
	m_current_scene->engine.input = input.get();

	m_current_scene->init();

	m_current_player_component = m_current_scene->get_first_component_of_type<PlayerComponent>();
}

GameApp::GameApp(GraphicsAPI_Type type) : App(type)
{
}

GameApp::~GameApp()
{
}

#if SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

void GameApp::run2()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Vulkan_LoadLibrary(nullptr);
	SDL_Window* window = SDL_CreateWindow("test", 640, 360, SDL_WINDOW_VULKAN);

	bool running = true;
	while (running) {
		SDL_Event windowEvent;
		while (SDL_PollEvent(&windowEvent))
			if (windowEvent.type == SDL_EVENT_QUIT) {
				running = false;
				break;
			}
	}

	SDL_DestroyWindow(window);
	SDL_Vulkan_UnloadLibrary();
	SDL_Quit();
}

#endif

void GameApp::init()
{
	QUE_PROFILE;

	PhysicsSystem::init_static();
	m_physics_system = std::make_unique<PhysicsSystem>();
	m_audio_system = std::make_unique<AudioSystem>();

#if defined(__ANDROID__)
	m_asset_manager = std::make_shared<AssetSystem>(androidApp->activity->assetManager, m_audio_system.get(), m_graphicsAPI.get());
#else
	m_asset_manager = std::make_shared<AssetSystem>(m_audio_system.get(), m_graphicsAPI.get());
#endif

	m_renderer = std::make_unique<Renderer>(m_graphicsAPI, m_colorSwapchainInfos, m_depthSwapchainInfos, m_registry);

	init_imgui();

	g_engine.asset = m_asset_manager.get();
	g_engine.audio = m_audio_system.get();
	g_engine.physics = m_physics_system.get();
	g_engine.reg = &m_registry;
	g_engine.render = m_renderer.get();
	g_engine.input = input.get();
	
	change_scene<GameScene>();
}

void GameApp::init_imgui()
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

void GameApp::update(float dt)
{
	QUE_PROFILE;

	m_physics_system->update(1.0f / 60.0f, m_registry);

	if (m_current_scene)
		m_current_scene->update();
}

void GameApp::render(FrameRenderInfo& info)
{
	QUE_PROFILE;

	m_current_player_component->pose = info.view.pose;

	if (m_current_player_component)
		m_renderer->render(m_current_player_component->entity->position, info);

	#pragma region Imgui

	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(32, 48), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Once);
	if (ImGui::Begin("Phys Debug", nullptr))
	{
		/*ImGui::Text("playerpos: %f %f", m_current_player_component->entity->position.x, m_current_player_component->entity->position.z);
		ImGui::Text("forward: %f %f %f", m_forward.x, m_forward.y, m_forward.z);*/

		ImGui::LabelText("Deltatime: ", "%f", m_delta_time);

		//if (ImGui::Button("Spawn object at hmd's position"))
		//{
		//	game::tmpl::create_block(m_registry, *m_physics_system, glm::vec3(0.0f, 5.0f, 0.0f), test_cube, nullptr);
		//}
	}
	ImGui::End();


	if (ImGui::Begin("ECS Debug", nullptr))
	{
		ImGui::Text("Models to render");
		// get all mesh components
		auto modelsToRender = m_registry.view<core_transform_component, core_mesh_component>();
		for (const auto&& [e, tc, mc] : modelsToRender.each())
		{
			ImGui::Text("Entity: %d", e);
			ImGui::Text("Position: %f %f %f", tc.position.x, tc.position.y, tc.position.z);
			ImGui::Text("Scale: %f %f %f", tc.scale.x, tc.scale.y, tc.scale.z);
		}
	}
	ImGui::End();

	// draw imgui debug
	if (ImGui::Begin("Phys debug"))
	{
		ImGui::Text("Bodies");
		// get all mesh components
		auto modelsToRender = m_registry.view<physics_component>();
		for (const auto&& [e, pc] : modelsToRender.each())
		{
			auto ppos = m_physics_system->get_body_position(pc.id);
			auto ptype = m_physics_system->get_body_type(pc.id);

			ImGui::Text("Body: %d", pc.id);
			ImGui::Text("Position: %f %f %f", ppos.x, ppos.y, ppos.z);
			ImGui::Text("Type: %d", (int)ptype);
		}

	}
	ImGui::End();

	input->draw_imgui();

	ImGui::Render();
#pragma endregion
}

void GameApp::destroy()
{
	m_graphicsAPI->MainDeletionQueue.execute();
}

