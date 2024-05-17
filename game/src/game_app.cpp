#include "pch.h"

#include "game_app.h"

#include <asset/mesh.h>

#include <core/asset.h>
#include <core/profiler.h>


#include <gfx/pipeline/sky_pipeline.h>
#include <gfx/pipeline/mesh_pipeline.h>
#include <gfx/pipeline/sky_cube_render_pipeline.h>
#include <gfx/sky.h>

#include <common/GraphicsAPI.h>
#include <common/vk_initializers.h>
#include <common/glm_helpers.h>

#include <game/systems/systems.h>
#include <game/tags.h>
#include <game/components.h>

GameApp::GameApp(GraphicsAPI_Type type) : App(type)
{
}

GameApp::~GameApp()
{
}

void GameApp::init()
{
	QUE_PROFILE;

	PhysicsSystem::init_static();
	m_physics_system = std::make_unique<PhysicsSystem>();

	m_renderer = new Renderer(m_graphicsAPI, m_colorSwapchainInfos, m_depthSwapchainInfos);

	init_imgui();


	create_resources();

	const auto entity = m_registry.create();
	m_registry.emplace<transform_component>(entity, glm::vec3{ 0.0f,-1.0f,0.0f }, glm::quat(1, 0, 0, 0), glm::vec3{ 0.5f, 0.5f, 0.5f });
	m_registry.emplace<mesh_component>(entity, level_model);

	const auto ball = m_registry.create();

	m_registry.emplace<transform_component>(ball, glm::vec3{ 0.0f,5.0f,0.0f }, glm::quat(1, 0, 0, 0), glm::vec3{ 0.5f, 0.5f, 0.5f });
	m_registry.emplace<mesh_component>(ball, test_cube);

	JPH::BodyCreationSettings obj_settings(
		new JPH::BoxShape(JPH::Vec3(0.5f, 0.5f, 0.5f)),
		JPH::RVec3(0, 5, 0),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Dynamic,
		Layers::MOVING);
	
	m_registry.emplace<physics_component>(ball, m_physics_system->spawn_body(obj_settings, JPH::Vec3(0, -1.0f, 0.0f)));

	// controllers
	const auto controller1 = m_registry.create();
	JPH::BodyCreationSettings c_settings(
		new JPH::SphereShape(0.5f),
		JPH::RVec3(0, 10.8, 0),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Kinematic,
		Layers::MOVING);

	m_registry.emplace<transform_component>(controller1, glm::vec3{ 0.0f,5.0f,0.0f }, glm::quat(1, 0, 0, 0), glm::vec3{ 0.5f, 0.5f, 0.5f });
	//m_registry.emplace<physics_component>(controller1, m_physics_system->spawn_body(c_settings, JPH::Vec3(0.7f, -1.0f, 0.1f)));
	m_registry.emplace<mesh_component>(controller1, controller);
	m_registry.emplace<controller_component>(controller1, 0);
	m_registry.emplace<main_interacting_controller>(controller1);
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

	game::system::update_physics_system(m_registry, *m_physics_system);
	game::system::update_controller_system(m_registry, *input, *m_physics_system, m_viewHeightM);
	game::system::update_block_pickup_system(m_registry, *input);

	m_physics_system->update(dt, m_registry);
}

void GameApp::render(FrameRenderInfo& info)
{
	QUE_PROFILE;

	m_renderer->render(info, m_registry);







	#pragma region Imgui

	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(32, 48), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Once);
	if (ImGui::Begin("Phys Debug", nullptr))
	{
		if (ImGui::Button("Spawn object at hmd's position"))
		{
			const auto ent = m_registry.create();
			m_registry.emplace<transform_component>(ent, glm::vec3(0.0f, 10.0f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f));
			m_registry.emplace<mesh_component>(ent, controller);

			auto pid = m_physics_system->spawn_body(JPH::BodyCreationSettings(
				new JPH::SphereShape(2.0f),
				JPH::RVec3(0, 10, 0),
				JPH::Quat::sIdentity(),
				JPH::EMotionType::Dynamic,
				Layers::MOVING), JPH::Vec3(0.0f, 1.0f, 1.0f));

			m_registry.emplace<physics_component>(ent, pid);
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

	ImGui::Render();
#pragma endregion
}

void GameApp::destroy()
{
	destroy_resources();
}

void GameApp::create_resources()
{
	QUE_PROFILE;

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