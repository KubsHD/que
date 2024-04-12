#include "game_app.h"

#include <asset/mesh.h>
#include <core/asset.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>
#include <lib/tiny_gltf.h>
#include <lib/stb_image.h>
#include <vulkan/vulkan.h>
#include <lib/imgui/imgui_impl_vulkan.h>



#include <common/GraphicsAPI.h>
#include <gfx/pipeline/sky_pipeline.h>
#include <gfx/pipeline/mesh_pipeline.h>
#include <entt/entt.hpp>

#include <gfx/sky.h>

#include <common/vk_initializers.h>
#include <gfx/pipeline/sky_cube_render_pipeline.h>
#include <common/glm_helpers.h>
#include <core/profiler.h>
#include <lib/netimgui/NetImgui_Api.h>
#include <game/components.h>

entt::registry registry;

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

	init_imgui();

	create_resources();

	const auto entity = m_registry.create();
	m_registry.emplace<transform_component>(entity, glm::vec3{ 0.0f,-1.0f,0.0f }, glm::quat(1, 0, 0, 0), glm::vec3{ 0.5f, 0.5f, 0.5f });
	m_registry.emplace<mesh_component>(entity, mod);

	const auto ball = m_registry.create();

	m_registry.emplace<transform_component>(ball, glm::vec3{ 0.0f,5.0f,0.0f }, glm::quat(1, 0, 0, 0), glm::vec3{ 0.5f, 0.5f, 0.5f });
	m_registry.emplace<mesh_component>(ball, controller);

	JPH::BodyCreationSettings obj_settings(
		new JPH::SphereShape(0.01f),
		JPH::RVec3(0, 10.8, 0),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Dynamic,
		Layers::MOVING);

	
	m_registry.emplace<rigidbody_component>(ball, m_physics_system->spawn_body(obj_settings, JPH::Vec3(0.7f, -1.0f, 0.1f)));
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

	m_physics_system->update(dt, m_registry);
}



int currently_drawn_object = 0;

void GameApp::render_model(glm::vec3 pos, glm::vec3 scale, glm::quat rot, const Model& model)
{
	gfx::InstanceData id;

	id.model = glm::mat4(1.0f);
	id.model = glm::translate(id.model, pos);
	id.model *= glm::mat4(rot);
	id.model = glm::scale(id.model, scale);

	size_t offsetCameraUB = sizeof(gfx::InstanceData) * currently_drawn_object;

	m_graphicsAPI->SetPipeline(m_pipeline);
	m_graphicsAPI->SetBufferData(m_instanceData, offsetCameraUB, sizeof(gfx::InstanceData), &id);

	for (auto mesh : model.meshes)
	{
		m_graphicsAPI->SetDescriptor({ 0, 0, m_sceneData, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, 0, sizeof(gfx::SceneData) });
		m_graphicsAPI->SetDescriptor({ 1, 0, m_instanceData, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, offsetCameraUB, sizeof(gfx::InstanceData) });
		m_graphicsAPI->SetDescriptor({ 1, 1, model.materials.at(mesh.material_index).diff.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false});
		m_graphicsAPI->SetDescriptor({ 1, 2, model.materials.at(mesh.material_index).norm.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });
		m_graphicsAPI->SetDescriptor({ 1, 3, model.materials.at(mesh.material_index).orm.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });
		m_graphicsAPI->SetDescriptor({ 1, 4, m_sky.skyCubemap.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });

		m_graphicsAPI->UpdateDescriptors();
		
		m_graphicsAPI->SetVertexBuffers(&mesh.vertex_buffer, 1);
		m_graphicsAPI->SetIndexBuffer(mesh.index_buffer);
		m_graphicsAPI->DrawIndexed(mesh.index_count);
	}
	
	currently_drawn_object++;
}


static float posx = 0;
static GPUModelConstant pushConst;

void GameApp::render(FrameRenderInfo& info)
{
	QUE_PROFILE;

	GraphicsAPI::Viewport viewport = { 0.0f, 0.0f, (float)info.width, (float)info.height, 0.0f, 1.0f };
	GraphicsAPI::Rect2D scissor = { {(int32_t)0, (int32_t)0}, {(uint32_t)info.width,(uint32_t)info.height} };
	float nearZ = 0.05f;
	float farZ = 100.0f;

	currently_drawn_object = 0;

	// Rendering code to clear the color and depth image views.
	m_graphicsAPI->BeginRendering();

	if (m_environmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE) {
		// VR mode use a background color.
		m_graphicsAPI->ClearColor(info.colorSwapchainInfo->imageViews[info.colorImageIndex], 0.17f, 0.17f, 0.17f, 1.00f);
	}
	else {
		// In AR mode make the background color black.
		m_graphicsAPI->ClearColor(info.colorSwapchainInfo->imageViews[info.colorImageIndex], 0.00f, 0.00f, 0.00f, 1.00f);
	}
	m_graphicsAPI->ClearDepth(info.depthSwapchainInfo->imageViews[info.depthImageIndex], 1.0f);

	m_graphicsAPI->SetRenderAttachments(info.colorSwapchainInfo->imageViews[info.colorImageIndex], 1, info.depthSwapchainInfo->imageViews[info.depthImageIndex], info.width, info.height, sky_pipeline);
	m_graphicsAPI->SetViewports(&viewport, 1);
	m_graphicsAPI->SetScissors(&scissor, 1);
	
	// Compute the view-projection transform.
	// All matrices (including OpenXR's) are column-major, right-handed.
	XrMatrix4x4f proj;
	XrMatrix4x4f_CreateProjectionFov(&proj, m_apiType, info.view.fov, nearZ, farZ);
	XrMatrix4x4f toView;
	XrVector3f scale1m{ 1.0f, 1.0f, 1.0f };
	XrVector3f offset{ 0.0f, 1.5f, 0.0f };
	XrVector3f final_camera_pos;
	XrVector3f_Add(&final_camera_pos, &info.view.pose.position, &offset);


	XrMatrix4x4f_CreateTranslationRotationScale(&toView, &final_camera_pos, & info.view.pose.orientation, & scale1m);
	XrMatrix4x4f view;
	XrMatrix4x4f viewProj;
	XrMatrix4x4f_InvertRigidBody(&view, &toView);
	XrMatrix4x4f_Multiply(&viewProj, &proj, &view);

	m_sceneDataCPU.camPos = glm::to_glm(info.view.pose.position);
	m_sceneDataCPU.viewProj = glm::to_glm(viewProj);
	m_sceneDataCPU.proj = glm::mat4(0.0f);

	m_graphicsAPI->SetBufferData(m_sceneData, 0, sizeof(gfx::SceneData), &m_sceneDataCPU);


	auto modelsToRender = m_registry.view<transform_component, mesh_component>();
	for (const auto&& [e, tc, mc] : modelsToRender.each())
	{
		render_model(tc.position, tc.scale, tc.rotation, mc.model);
	};


	auto physEntities = m_registry.view<transform_component, rigidbody_component>();
	for (const auto&& [e, tc, rc] : physEntities.each())
	{
		tc.position = m_physics_system->get_body_position(rc.id);
	};


	for (auto& pose : input->get_controller_poses())
	{

		glm::vec3 target_pos = glm::to_glm(pose.position);
		target_pos += glm::vec3{ 0, m_viewHeightM, 0 };

		glm::quat xr_source_rotation = glm::to_glm(pose.orientation);
		glm::quat rot = glm::rotate(xr_source_rotation, glm::radians(180.0f), glm::vec3(0,1,0));

		render_model(target_pos, { 0.01f, 0.01f, 0.01f }, rot, controller);
	}

	// draw skybox
	XrVector3f pos = info.view.pose.position;
	XrVector3f scale = { 5.0f, 5.0f, 5.0f };
	XrQuaternionf rot = { 0.0f, 0.0f, 0.0f };

	XrMatrix4x4f_CreateTranslationRotationScale(&pushConst.model, &pos, &rot, &scale);

	m_graphicsAPI->SetPipeline(sky_pipeline);

	m_graphicsAPI->PushConstant(&pushConst, sizeof(GPUModelConstant));

	m_graphicsAPI->SetDescriptor({0,  0, m_sceneData, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, 0, sizeof(gfx::SceneData) });
	m_graphicsAPI->SetDescriptor({0,  1, skybox_image.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });
	
	m_graphicsAPI->UpdateDescriptors();

	for (auto mesh : skybox_cube.meshes)
	{
		m_graphicsAPI->SetVertexBuffers(&mesh.vertex_buffer, 1);
		m_graphicsAPI->SetIndexBuffer(mesh.index_buffer);
		m_graphicsAPI->DrawIndexed(mesh.index_count);
	}

	m_graphicsAPI->EndRendering();

	
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(32, 48), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Once);
	if (ImGui::Begin("Phys Debug", nullptr))
	{
		if (ImGui::Button("Spawn object at hmd's position"))
		{

			JPH::BodyCreationSettings obj_settings(
				new JPH::SphereShape(0.01f),
				JPH::RVec3(0, 10.8, 0),
				JPH::Quat::sIdentity(),
				JPH::EMotionType::Dynamic,
				Layers::MOVING);
			m_physics_system->spawn_body(obj_settings, JPH::Vec3(0.7f, -1.0f, 0.1f));
		}
	}
	ImGui::End();


	if (ImGui::Begin("ECS Debug", nullptr))
	{
	
	}
	ImGui::End();

	ImGui::Render();
}

void GameApp::destroy()
{
	destroy_resources();
}

void GameApp::create_resources()
{
	QUE_PROFILE;

	// per scene constants
	m_sceneData = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(gfx::SceneData), nullptr });
	
	// per instance constants (allocating a lot for now)
	// todo: make this dynamic
	m_instanceData = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(gfx::InstanceData) * 1024, nullptr });

	m_pipeline = pipeline::create_mesh_pipeline(*m_graphicsAPI, m_asset_manager, (VkFormat)m_colorSwapchainInfos[0].swapchainFormat, (VkFormat)m_depthSwapchainInfos[0].swapchainFormat);
	m_sky_render_pipeline = pipeline::create_sky_cube_render_pipeline(*m_graphicsAPI, m_asset_manager);
	
	mod = Asset::load_model(*m_graphicsAPI, "data/level/testlevel.gltf");
	skybox_cube = Asset::load_model(*m_graphicsAPI, "data/cube.gltf");
	controller = Asset::load_model_json(*m_graphicsAPI, "data/models/meta/model_controller_left.model");
	skybox_image = Asset::load_image(*m_graphicsAPI, "data/apartment.hdr", TT_HDRI);

	sky_pipeline = pipeline::create_sky_pipeline(*m_graphicsAPI, m_asset_manager, (VkFormat)m_colorSwapchainInfos[0].swapchainFormat, (VkFormat)m_depthSwapchainInfos[0].swapchainFormat);
	m_sky = gfx::sky::create_sky(*m_graphicsAPI, "data/apartment.hdr", skybox_cube, m_sky_render_pipeline);

	auto device = m_graphicsAPI->GetDevice();

	// create sampler
	VkSamplerCreateInfo sinfo = vkinit::sampler_create_info(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	VULKAN_CHECK_NOMSG(vkCreateSampler(device, &sinfo, nullptr, &sampler));
	
	m_graphicsAPI->MainDeletionQueue.push_function([&]() {
		vkDestroySampler(m_graphicsAPI->GetDevice(), sampler, nullptr);

		m_graphicsAPI->DestroyImage(skybox_image.image);
		m_graphicsAPI->DestroyImageView(skybox_image.view);
		vmaFreeMemory(m_graphicsAPI->GetAllocator(), skybox_image.allocation);

		for (auto m : mod.meshes)
		{
			m_graphicsAPI->DestroyBuffer(m.index_buffer);
			m_graphicsAPI->DestroyBuffer(m.vertex_buffer);
	
		}

		for (auto m : skybox_cube.meshes)
		{
			m_graphicsAPI->DestroyBuffer(m.index_buffer);
			m_graphicsAPI->DestroyBuffer(m.vertex_buffer);
		}

		m_graphicsAPI->DestroyPipeline(sky_pipeline);
		m_graphicsAPI->DestroyBuffer(m_sceneData);
		m_graphicsAPI->DestroyPipeline(m_pipeline);
		
	});
}

void GameApp::destroy_resources()
{
	m_graphicsAPI->MainDeletionQueue.execute();
}