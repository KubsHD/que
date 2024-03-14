#include "game_app.h"

#include <asset/mesh.h>
#include <core/asset.h>


#include <glm/ext.hpp>
#include <lib/tiny_gltf.h>

#include <lib/stb_image.h>

#include <vulkan/vulkan.h>
#include <common/GraphicsAPI.h>
#include <pipeline/sky_pipeline.h>

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

void GameApp::render_cube(XrPosef pose, XrVector3f scale, XrVector3f color)
{
}

void GameApp::render_mesh(XrVector3f pos, XrVector3f scale, XrQuaternionf rot, const Model& model)
{
	XrMatrix4x4f_CreateTranslationRotationScale(&cameraConstants.model, &pos, &rot, &scale);

	XrMatrix4x4f_Multiply(&cameraConstants.modelViewProj, &cameraConstants.viewProj, &cameraConstants.model);
	size_t offsetCameraUB = sizeof(CameraConstants) *0;

	m_graphicsAPI->SetPipeline(m_pipeline);

	m_graphicsAPI->SetBufferData(m_uniformBuffer_Camera, offsetCameraUB, sizeof(CameraConstants), &cameraConstants);
	m_graphicsAPI->SetDescriptor({ 0, m_uniformBuffer_Camera, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, offsetCameraUB, sizeof(CameraConstants) });
	m_graphicsAPI->SetDescriptor({ 1, model_texture.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });

	m_graphicsAPI->UpdateDescriptors();

	for (auto mesh : model.meshes)
	{
		m_graphicsAPI->SetVertexBuffers(&mesh.vertex_buffer, 1);
		m_graphicsAPI->SetIndexBuffer(mesh.index_buffer);
		m_graphicsAPI->DrawIndexed(mesh.index_count);
	}
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



	m_graphicsAPI->SetRenderAttachments(info.colorSwapchainInfo->imageViews[info.colorImageIndex], 1, info.depthSwapchainInfo->imageViews[info.depthImageIndex], info.width, info.height, m_pipeline);
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
	XrMatrix4x4f_Multiply(&cameraConstants.viewProj, &proj, &view);

	render_mesh({ posx, -1.0f, 0.0f}, { 0.1f, 0.1f, 0.1f }, { 0.0f, 0.0f, 0.0f }, mod);

	// draw skybox
	XrVector3f pos = { 0.0f, 0.0f, 0.0f };
	XrVector3f scale = { 20.0f, 20.0f, 20.0f };
	XrQuaternionf rot = { 0.0f, 0.0f, 0.0f };

	XrMatrix4x4f_CreateTranslationRotationScale(&pushConst.model, &pos, &rot, &scale);

	m_graphicsAPI->SetPipeline(sky_pipeline);

	m_graphicsAPI->PushConstant(&pushConst, sizeof(GPUModelConstant));

	m_graphicsAPI->SetBufferData(m_uniformBuffer_Camera, 0, sizeof(CameraConstants), &cameraConstants);
	m_graphicsAPI->SetDescriptor({ 0, m_uniformBuffer_Camera, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, 0, sizeof(CameraConstants) });
	m_graphicsAPI->SetDescriptor({ 1, skybox_image.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });

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
}

void GameApp::create_resources()
{
	std::vector<char> vertexSource = m_asset_manager->read_all_bytes("data/shader/mesh.vert.spv");
	m_vertexShader = m_graphicsAPI->CreateShader({ GraphicsAPI::ShaderCreateInfo::Type::VERTEX, vertexSource.data(), vertexSource.size() });

	std::vector<char> fragmentSource = m_asset_manager->read_all_bytes("data/shader/mesh.frag.spv");
	m_fragmentShader = m_graphicsAPI->CreateShader({ GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT, fragmentSource.data(), fragmentSource.size() });

	size_t numberOfCuboids = 2;
	m_uniformBuffer_Camera = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(CameraConstants) * numberOfCuboids, nullptr });

	GraphicsAPI::PipelineCreateInfo pipelineCI;

	pipelineCI.shaders = { m_vertexShader, m_fragmentShader };
	pipelineCI.vertexInputState.attributes = { 
		{0, 0, GraphicsAPI::VertexType::VEC3, 0, "POSITION"},  
		{1, 0, GraphicsAPI::VertexType::VEC3, offsetof(Vertex, nx), "NORMAL"},
		{2, 0, GraphicsAPI::VertexType::VEC2, offsetof(Vertex, u), "TEXCOORD"}
	};
													// stride VVVV ie. sizeof(Vertex)
	pipelineCI.vertexInputState.bindings = { {0, 0, 8 * sizeof(float)} };
	pipelineCI.inputAssemblyState = { GraphicsAPI::PrimitiveTopology::TRIANGLE_LIST, false };
	pipelineCI.rasterisationState = { false, false, GraphicsAPI::PolygonMode::FILL, 
									GraphicsAPI::CullMode::FRONT, GraphicsAPI::FrontFace::CLOCKWISE,
									false, 0.0f, 0.0f, 0.0f, 1.0f };
	pipelineCI.multisampleState = { 1, false, 1.0f, 0xFFFFFFFF, false, false };
	pipelineCI.depthStencilState = { true, true, GraphicsAPI::CompareOp::LESS_OR_EQUAL, false, false, {}, {}, 0.0f, 1.0f };
	pipelineCI.colorBlendState = { false, GraphicsAPI::LogicOp::NO_OP, {{true, GraphicsAPI::BlendFactor::SRC_ALPHA, GraphicsAPI::BlendFactor::ONE_MINUS_SRC_ALPHA, GraphicsAPI::BlendOp::ADD, GraphicsAPI::BlendFactor::ONE, GraphicsAPI::BlendFactor::ZERO, GraphicsAPI::BlendOp::ADD, (GraphicsAPI::ColorComponentBit)15}}, {0.0f, 0.0f, 0.0f, 0.0f} };
	pipelineCI.colorFormats = { m_colorSwapchainInfos[0].swapchainFormat };
	pipelineCI.depthFormat = m_depthSwapchainInfos[0].swapchainFormat;
	pipelineCI.layout = { {0, nullptr, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
						  {1, nullptr, nullptr, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT} };
	
	m_pipeline = m_graphicsAPI->CreatePipeline(pipelineCI);

	mod = Asset::load_model(*m_graphicsAPI, "data/backpack.obj");
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

	sky_pipeline = pipeline::create_sky_pipeline(*m_graphicsAPI, m_asset_manager, (VkFormat)m_colorSwapchainInfos[0].swapchainFormat, (VkFormat)m_depthSwapchainInfos[0].swapchainFormat);
}


void GameApp::destroy_resources()
{
	m_graphicsAPI->DestroyPipeline(m_pipeline);
	m_graphicsAPI->DestroyShader(m_fragmentShader);
	m_graphicsAPI->DestroyShader(m_vertexShader);
	m_graphicsAPI->DestroyBuffer(m_uniformBuffer_Camera);
}