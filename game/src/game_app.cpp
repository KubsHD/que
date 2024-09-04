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

#include <game/tags.h>
#include <game/components.h>
#include <game/systems/systems.h>
#include <game/templates/controller_template.h>
#include <game/templates/block_template.h>
#include <game/systems/engine/attach_system.h>
#include <game/components/speaker_interactable.h>

#include <core/physics_util.h>

GameApp::GameApp(GraphicsAPI_Type type) : App(type)
{
}

GameApp::~GameApp()
{
}

#if SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <core/physics_util.h>
#include <common/serialization.h>
#include <game/components/speaker_interactable.h>
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
	m_asset_manager = std::make_shared<Asset>(androidApp->activity->assetManager, m_audio_system.get());

#else
	m_asset_manager = std::make_shared<Asset>(m_audio_system.get());
#endif

	m_renderer = new Renderer(m_graphicsAPI, m_colorSwapchainInfos, m_depthSwapchainInfos);

	init_imgui();
	create_resources();
	init_game_world();

	m_audio_system->play_sound(*bgm);

	for (auto& mesh : level_model.meshes)
	{
		JPH::BodyCreationSettings obj_settings(
			core::physics::create_mesh_shape(mesh),
			JPH::RVec3(0, -1.0f, 0),
			JPH::Quat::sIdentity(),
			JPH::EMotionType::Static,
			Layers::NON_MOVING);

		obj_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
		obj_settings.mMassPropertiesOverride.mMass = 100.0f;
		obj_settings.mMassPropertiesOverride.mInertia = JPH::Mat44::sIdentity();

		m_physics_system->spawn_body(obj_settings);
	}



	const auto entity = m_registry.create();
	m_registry.emplace<transform_component>(entity, glm::vec3{ 0.0f,-1.0f,0.0f }, glm::quat(1, 0, 0, 0), glm::vec3{1.0f, 1.0f, 1.0f });
	m_registry.emplace<mesh_component>(entity, level_model);

	// controllers
	game::tmpl::create_controller(m_registry, controller, 0);
	game::tmpl::create_controller(m_registry, controller, 1); 

	game::tmpl::create_block(m_registry, *m_physics_system, glm::vec3(0, 2, 0), test_cube, nullptr);


	// load saved objects
	load_saved_objects();
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


void GameApp::init_game_world()
{
	m_registry.ctx().emplace<engine>(*m_audio_system, *m_asset_manager, *m_physics_system);

	m_registry.on_construct<attach_component>().connect<&game::system::on_attach_component_created>();
	m_registry.on_destroy<attach_component>().connect<&game::system::on_attach_component_destroyed>();

	m_registry.on_construct<speaker_interactable>().connect<&game::system::on_speaker_interact>();

	//m_registry.on_construct<physics_component>().connect();
}

void GameApp::load_saved_objects()
{
	auto so = Asset::Instance->read_json("data/saved_objects.json");

	for (auto obj : so["objects"])
	{
		models.push_back(Asset::load_model_json(*m_graphicsAPI, obj["model"]));

		JPH::RefConst<JPH::Shape> shape = nullptr;

		if (obj.contains("physics"))
			shape = core::physics::load_from_file(obj["physics"]);

		auto b = game::tmpl::create_block(m_registry, *m_physics_system, ser::vec3_deserialize(obj["position"]), models.back(), shape);
		m_registry.emplace<saveable>(b);
	}

}

void GameApp::save_objects()
{
}

void GameApp::update(float dt)
{
	QUE_PROFILE;

	// test
	game::system::update_physics_system(m_registry, *m_physics_system);
	game::system::update_controller_system(m_registry, *input, *m_physics_system, m_viewHeightM, player_pos);
	game::system::update_block_pickup_system(m_registry, *input, *m_physics_system);
	game::system::update_attach_system(m_registry);

	m_physics_system->update(1.0f / 60.0f, m_registry);
}

void GameApp::render(FrameRenderInfo& info)
{
	QUE_PROFILE;

	auto vel = input->get_movement_input();
	m_forward = glm::normalize(glm::to_glm(info.view.pose.orientation)) * glm::vec3(0.0f, 0.0f, -1.0f);
	auto right = glm::normalize(glm::to_glm(info.view.pose.orientation)) * glm::vec3(1.0f, 0.0f, 0.0f);;
	auto forward = m_forward;


	player_pos = player_pos + (vel.y * forward * m_speed) + vel.x * right * m_speed;

	m_renderer->render(player_pos, info, m_registry);

	#pragma region Imgui

	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(32, 48), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Once);
	if (ImGui::Begin("Phys Debug", nullptr))
	{
		ImGui::Text("playerpos: %f %f", player_pos.x, player_pos.z);
		ImGui::Text("forward: %f %f %f", m_forward.x, m_forward.y, m_forward.z);

		ImGui::LabelText("Deltatime: ", "%f", m_delta_time);

		if (ImGui::Button("Spawn object at hmd's position"))
		{
			game::tmpl::create_block(m_registry, *m_physics_system, glm::vec3(0.0f, 5.0f, 0.0f), test_cube, nullptr);
		}
	}
	ImGui::End();


	if (ImGui::Begin("ECS Debug", nullptr))
	{
		ImGui::Text("Models to render");
		// get all mesh components
		auto modelsToRender = m_registry.view<transform_component, mesh_component>();
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
	save_objects();

	destroy_resources();
}

void GameApp::create_resources()
{
	QUE_PROFILE;

	bgm = Asset::load_sound("data/audio/background_music_1.mp3");

	level_model = Asset::load_model(*m_graphicsAPI, "data/level/testlevel.gltf");
	skybox_cube = Asset::load_model(*m_graphicsAPI, "data/cube.gltf");
	controller = Asset::load_model_json(*m_graphicsAPI, "data/models/meta/model_controller_left.model");
	skybox_image = Asset::load_image(*m_graphicsAPI, "data/apartment.hdr", TT_HDRI);
	test_cube = Asset::load_model_json(*m_graphicsAPI, "data/models/blocks/brick/Cube.model");
}

void GameApp::destroy_resources()
{
	m_graphicsAPI->MainDeletionQueue.execute();
}