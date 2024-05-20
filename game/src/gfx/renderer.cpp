#include "pch.h"

#include "renderer.h"


#include <asset/mesh.h>

#include <core/asset.h>
#include <core/profiler.h>


#include <gfx/pipeline/sky_pipeline.h>
#include <gfx/pipeline/mesh_pipeline.h>
#include <gfx/pipeline/sky_cube_render_pipeline.h>
#include <gfx/pipeline/sky_irradiance_generate.h>

#include <gfx/sky.h>

#include <common/GraphicsAPI.h>
#include <common/GraphicsAPI_Vulkan.h>
#include <common/vk_initializers.h>
#include <common/glm_helpers.h>


#include <game/systems/systems.h>
#include <game/tags.h>
#include <game/components.h>
#include <app.h>
#include "pipeline/unlit_mesh_pipeline.h"

Renderer::Renderer(std::shared_ptr<GraphicsAPI_Vulkan> gapi, std::vector<App::SwapchainInfo> colorFormats, std::vector<App::SwapchainInfo> depthFormats) : m_colorSwapchainInfos(colorFormats), m_depthSwapchainInfos(depthFormats), m_graphicsAPI(gapi)
{
	create_resources();
}

Renderer::~Renderer()
{
	destroy_resources();
}

void Renderer::render(App::FrameRenderInfo& info, entt::registry& reg)
{
	currently_drawn_object = 0;

	GraphicsAPI::Viewport viewport = { 0.0f, 0.0f, (float)info.width, (float)info.height, 0.0f, 1.0f };
	GraphicsAPI::Rect2D scissor = { {(int32_t)0, (int32_t)0}, {(uint32_t)info.width,(uint32_t)info.height} };
	float nearZ = 0.05f;
	float farZ = 100.0f;

	// Rendering code to clear the color and depth image views.
	m_graphicsAPI->BeginRendering();

	m_graphicsAPI->ClearColor(info.colorSwapchainInfo->imageViews[info.colorImageIndex], 0.17f, 0.17f, 0.17f, 1.00f);

	m_graphicsAPI->ClearDepth(info.depthSwapchainInfo->imageViews[info.depthImageIndex], 1.0f);

	m_graphicsAPI->SetRenderAttachments(info.colorSwapchainInfo->imageViews[info.colorImageIndex], 1, 
		info.depthSwapchainInfo->imageViews[info.depthImageIndex], info.width, info.height, m_sky_pipeline);
	m_graphicsAPI->SetViewports(&viewport, 1);
	m_graphicsAPI->SetScissors(&scissor, 1);

	// Compute the view-projection transform.
	// All matrices (including OpenXR's) are column-major, right-handed.
	XrMatrix4x4f proj;
	XrMatrix4x4f_CreateProjectionFov(&proj, GraphicsAPI_Type::VULKAN, info.view.fov, nearZ, farZ);
	XrMatrix4x4f toView;
	XrVector3f scale1m{ 1.0f, 1.0f, 1.0f };
	XrVector3f offset{ 0.0f, 1.5f, 0.0f };
	XrVector3f final_camera_pos;
	XrVector3f_Add(&final_camera_pos, &info.view.pose.position, &offset);

	XrMatrix4x4f_CreateTranslationRotationScale(&toView, &final_camera_pos, &info.view.pose.orientation, &scale1m);
	XrMatrix4x4f view;
	XrMatrix4x4f viewProj;
	XrMatrix4x4f_InvertRigidBody(&view, &toView);
	XrMatrix4x4f_Multiply(&viewProj, &proj, &view);


	m_sceneDataCPU.camPos = glm::to_glm(info.view.pose.position);
	m_sceneDataCPU.viewProj = glm::to_glm(viewProj);
	m_sceneDataCPU.proj = glm::mat4(0.0f);
	m_sceneDataCPU.view = glm::mat4(0.0f);

	m_graphicsAPI->SetBufferData(m_sceneData, 0, sizeof(gfx::SceneData), &m_sceneDataCPU);


	auto modelsToRender = reg.view<transform_component, mesh_component>();
	for (const auto&& [e, tc, mc] : modelsToRender.each())
	{
		render_model(tc.position, tc.scale, tc.rotation, mc.model);
	};

	draw_sky(info);

	m_graphicsAPI->EndRendering();
}

void Renderer::render_model(glm::vec3 pos, glm::vec3 scale, glm::quat rot, const Model& model)
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
		m_graphicsAPI->SetDescriptor({ 1, 1, model.materials.at(mesh.material_index).diff.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });
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

void Renderer::draw_sky(App::FrameRenderInfo& info)
{
	// draw skybox
	XrVector3f pos = info.view.pose.position;
	XrVector3f scale = { 5.0f, 5.0f, 5.0f };
	XrQuaternionf rot = { 0.0f, 0.0f, 0.0f };

	XrMatrix4x4f_CreateTranslationRotationScale(&pushConst.model, &pos, &rot, &scale);

	m_graphicsAPI->SetPipeline(m_sky_pipeline);

	m_graphicsAPI->PushConstant(&pushConst, sizeof(GPUModelConstant));

	m_graphicsAPI->SetDescriptor({ 0,  0, m_sceneData, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, 0, sizeof(gfx::SceneData) });
	m_graphicsAPI->SetDescriptor({ 0,  1, m_sky.skyIrradiance.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });

	m_graphicsAPI->UpdateDescriptors();

	for (auto mesh : skybox_cube.meshes)
	{
		m_graphicsAPI->SetVertexBuffers(&mesh.vertex_buffer, 1);
		m_graphicsAPI->SetIndexBuffer(mesh.index_buffer);
		m_graphicsAPI->DrawIndexed(mesh.index_count);
	}

}

void Renderer::create_resources()
{
	QUE_PROFILE;

	// per scene constants
	m_sceneData = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(gfx::SceneData), nullptr });

	// per instance constants (allocating a lot for now)
	// todo: make this dynamic
	m_instanceData = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(gfx::InstanceData) * 1024, nullptr });

	m_pipeline = pipeline::create_mesh_pipeline(*m_graphicsAPI, (VkFormat)m_colorSwapchainInfos[0].swapchainFormat, (VkFormat)m_depthSwapchainInfos[0].swapchainFormat);
	m_sky_render_pipeline = pipeline::create_sky_cube_render_pipeline(*m_graphicsAPI);
	m_unlit_pipeline = pipeline::create_unlit_mesh_pipeline(*m_graphicsAPI, (VkFormat)m_colorSwapchainInfos[0].swapchainFormat, (VkFormat)m_depthSwapchainInfos[0].swapchainFormat);

	skybox_cube = Asset::load_model(*m_graphicsAPI, "data/cube.gltf");

	m_sky_pipeline = pipeline::create_sky_pipeline(*m_graphicsAPI, (VkFormat)m_colorSwapchainInfos[0].swapchainFormat, (VkFormat)m_depthSwapchainInfos[0].swapchainFormat);
	m_sky = gfx::sky::create_sky(*m_graphicsAPI, "data/apartment.hdr", skybox_cube, m_sky_render_pipeline);

	auto device = m_graphicsAPI->GetDevice();

	// create sampler
	VkSamplerCreateInfo sinfo = vkinit::sampler_create_info(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	VULKAN_CHECK_NOMSG(vkCreateSampler(device, &sinfo, nullptr, &sampler));

	m_graphicsAPI->MainDeletionQueue.push_function([&]() {
		vkDestroySampler(m_graphicsAPI->GetDevice(), sampler, nullptr);

		for (auto m : skybox_cube.meshes)
		{
			m_graphicsAPI->DestroyBuffer(m.index_buffer);
			m_graphicsAPI->DestroyBuffer(m.vertex_buffer);
		}

		m_graphicsAPI->DestroyPipeline(m_sky_render_pipeline);
		m_graphicsAPI->DestroyBuffer(m_sceneData);
		m_graphicsAPI->DestroyPipeline(m_pipeline);

		});
}

void Renderer::destroy_resources()
{
	m_graphicsAPI->MainDeletionQueue.execute();
}
