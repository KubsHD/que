#include "game_app.h"

#include <asset/mesh.h>
#include <core/asset.h>


#include <glm/ext.hpp>
#include <lib/tiny_gltf.h>

#include <lib/stb_image.h>

#include <vulkan/vulkan.h>
#include <common/GraphicsAPI.h>
#include <pipeline/sky_pipeline.h>
#include <pipeline/mesh_pipeline.h>

GameApp::GameApp(GraphicsAPI_Type type) : App(type)
{
}

GameApp::~GameApp()
{
}

void GameApp::init()
{
	create_resources();
}

void GameApp::update()
{
}

Model mod;
GraphicsAPI::Image model_texture;

Model skybox_cube;
GraphicsAPI::Image skybox_image;

VkPipeline sky_pipeline;
VkSampler sampler;

int currently_drawn_object = 0;

void GameApp::render_mesh(XrVector3f pos, XrVector3f scale, XrQuaternionf rot, const Model& model)
{
	InstanceData id;

	XrMatrix4x4f_CreateTranslationRotationScale(&id.model, &pos, &rot, &scale);
	size_t offsetCameraUB = sizeof(InstanceData) * currently_drawn_object;

	m_graphicsAPI->SetPipeline(m_pipeline);

	m_graphicsAPI->SetBufferData(m_instanceData, offsetCameraUB, sizeof(InstanceData), &id);

	
	for (auto mesh : model.meshes)
	{
	
		m_graphicsAPI->SetDescriptor({ 0, 0, m_sceneData, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, 0, sizeof(SceneData) });
		m_graphicsAPI->SetDescriptor({ 1, 0, m_instanceData, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, offsetCameraUB, sizeof(InstanceData) });
		m_graphicsAPI->SetDescriptor({ 1, 1, mod.materials[mesh.material_index].diff.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false});
		m_graphicsAPI->SetDescriptor({ 1, 2, mod.materials[mesh.material_index].diff.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });
		m_graphicsAPI->SetDescriptor({ 1, 3, mod.materials[mesh.material_index].diff.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });

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
	GraphicsAPI::Viewport viewport = { 0.0f, 0.0f, (float)info.width, (float)info.height, 0.0f, 1.0f };
	GraphicsAPI::Rect2D scissor = { {(int32_t)0, (int32_t)0}, {(uint32_t)info.width,(uint32_t)info.height} };
	float nearZ = 0.05f;
	float farZ = 100.0f;

	//posx += 0.001f;
	
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
	XrMatrix4x4f_CreateTranslationRotationScale(&toView, &info.view.pose.position, &info.view.pose.orientation, &scale1m);
	XrMatrix4x4f view;
	XrMatrix4x4f_InvertRigidBody(&view, &toView);
	XrMatrix4x4f_Multiply(&m_sceneDataCPU.viewProj, &proj, &view);

	render_mesh({ posx, -1.0f, 0.0f}, { 0.1f, 0.1f, 0.1f }, { 0.0f, 0.0f, 0.0f }, mod);

	// draw skybox
	XrVector3f pos = info.view.pose.position;
	XrVector3f scale = { 5.0f, 5.0f, 5.0f };
	XrQuaternionf rot = { 0.0f, 0.0f, 0.0f };

	XrMatrix4x4f_CreateTranslationRotationScale(&pushConst.model, &pos, &rot, &scale);

	m_graphicsAPI->SetPipeline(sky_pipeline);
	m_graphicsAPI->SetBufferData(m_sceneData, 0, sizeof(SceneData), &m_sceneDataCPU);

	m_graphicsAPI->PushConstant(&pushConst, sizeof(GPUModelConstant));

	m_graphicsAPI->SetDescriptor({0,  0, m_sceneData, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, 0, sizeof(SceneData) });
	m_graphicsAPI->SetDescriptor({0,  1, skybox_image.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });
	
	m_graphicsAPI->UpdateDescriptors();

	for (auto mesh : skybox_cube.meshes)
	{
		m_graphicsAPI->SetVertexBuffers(&mesh.vertex_buffer, 1);
		m_graphicsAPI->SetIndexBuffer(mesh.index_buffer);
		m_graphicsAPI->DrawIndexed(mesh.index_count);
	}



	m_graphicsAPI->EndRendering();
}

void GameApp::destroy()
{
	destroy_resources();
}

void GameApp::create_resources()
{
	// per scene constants
	m_sceneData = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(SceneData), nullptr });
	
	// per instance constants (allocating a lot for now)
	// todo: make this dynamic
	m_instanceData = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(InstanceData) * 1024, nullptr });

	m_pipeline = pipeline::create_mesh_pipeline(*m_graphicsAPI, m_asset_manager, (VkFormat)m_colorSwapchainInfos[0].swapchainFormat, (VkFormat)m_depthSwapchainInfos[0].swapchainFormat);
	sky_pipeline = pipeline::create_sky_pipeline(*m_graphicsAPI, m_asset_manager, (VkFormat)m_colorSwapchainInfos[0].swapchainFormat, (VkFormat)m_depthSwapchainInfos[0].swapchainFormat);

	mod = Asset::load_model(*m_graphicsAPI, "data/testlevel.gltf");
	skybox_cube = Asset::load_model(*m_graphicsAPI, "data/cube.gltf");

	model_texture = Asset::load_image(*m_graphicsAPI, "data/diffuse.jpg", false);
	skybox_image = Asset::load_image(*m_graphicsAPI, "data/apartment.hdr", true);

	auto device = m_graphicsAPI->GetDevice();

	// create sampler
	VkSamplerCreateInfo sinfo = {};
	sinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sinfo.pNext = nullptr;

	sinfo.magFilter = VK_FILTER_NEAREST;
	sinfo.minFilter = VK_FILTER_NEAREST;
	sinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	VULKAN_CHECK_NOMSG(vkCreateSampler(*device, &sinfo, nullptr, &sampler));


	
	m_graphicsAPI->MainDeletionQueue.push_function([&]() {
		vkDestroySampler(*m_graphicsAPI->GetDevice(), sampler, nullptr);

		m_graphicsAPI->DestroyImage(model_texture.image);
		m_graphicsAPI->DestroyImageView(model_texture.view);
		vmaFreeMemory(*m_graphicsAPI->GetAllocator(), model_texture.allocation);

		m_graphicsAPI->DestroyImage(skybox_image.image);
		m_graphicsAPI->DestroyImageView(skybox_image.view);
		vmaFreeMemory(*m_graphicsAPI->GetAllocator(), skybox_image.allocation);

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