#include "game_app.h"

#include <glm/ext.hpp>

#include <lib/tiny_gltf.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

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



GameApp::Mesh m;

void GameApp::render_cube(XrPosef pose, XrVector3f scale, XrVector3f color)
{

}

void GameApp::render_mesh(XrVector3f pos, XrVector3f scale, XrQuaternionf rot, const Mesh& mesh)
{
	XrMatrix4x4f_CreateTranslationRotationScale(&cameraConstants.model, &pos, &rot, &scale);

	XrMatrix4x4f_Multiply(&cameraConstants.modelViewProj, &cameraConstants.viewProj, &cameraConstants.model);
	size_t offsetCameraUB = sizeof(CameraConstants) * renderCuboidIndex;

	m_graphicsAPI->SetPipeline(m_pipeline);

	m_graphicsAPI->SetBufferData(m_uniformBuffer_Camera, offsetCameraUB, sizeof(CameraConstants), &cameraConstants);
	m_graphicsAPI->SetDescriptor({ 0, m_uniformBuffer_Camera, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, offsetCameraUB, sizeof(CameraConstants) });

	m_graphicsAPI->UpdateDescriptors();

	m_graphicsAPI->SetVertexBuffers((void**)&m.vertex_buffer, 1);
	m_graphicsAPI->SetIndexBuffer(m.index_buffer);
	m_graphicsAPI->DrawIndexed(m.index_count);
}

void GameApp::render(FrameRenderInfo& info)
{
	GraphicsAPI::Viewport viewport = { 0.0f, 0.0f, (float)info.width, (float)info.height, 0.0f, 1.0f };
	GraphicsAPI::Rect2D scissor = { {(int32_t)0, (int32_t)0}, {(uint32_t)info.width,(uint32_t)info.height} };
	float nearZ = 0.05f;
	float farZ = 100.0f;

	

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

	m_graphicsAPI->SetRenderAttachments(&info.colorSwapchainInfo->imageViews[info.colorImageIndex], 1, info.depthSwapchainInfo->imageViews[info.depthImageIndex], info.width, info.height, m_pipeline);
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


	renderCuboidIndex = 0;
	// Draw a floor. Scale it by 2 in the X and Z, and 0.1 in the Y,
	render_mesh({0.0f, -1.0f, 0.0f}, { 2.0f, 0.1f, 2.0f }, { 0.0f, 0.0f, 0.0f }, m);
	// Draw a "table".
	//render_cube({ {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM + 0.9f, -0.7f} }, { 1.0f, 0.2f, 1.0f }, { 0.6f, 0.6f, 0.4f });

	m_graphicsAPI->EndRendering();

	
}

void GameApp::destroy()
{
}


void GameApp::create_resources()
{
	if (m_apiType == VULKAN) {
		std::vector<char> vertexSource = m_asset_manager->read_all_bytes("data/VertexShader.spv");
		m_vertexShader = m_graphicsAPI->CreateShader({ GraphicsAPI::ShaderCreateInfo::Type::VERTEX, vertexSource.data(), vertexSource.size() });

		std::vector<char> fragmentSource = m_asset_manager->read_all_bytes("data/PixelShader.spv");
		m_fragmentShader = m_graphicsAPI->CreateShader({ GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT, fragmentSource.data(), fragmentSource.size() });
	}

	size_t numberOfCuboids = 2;
	m_uniformBuffer_Camera = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(CameraConstants) * numberOfCuboids, nullptr });

	GraphicsAPI::PipelineCreateInfo pipelineCI;

	pipelineCI.shaders = { m_vertexShader, m_fragmentShader };
	pipelineCI.vertexInputState.attributes = { 
		{0, 0, GraphicsAPI::VertexType::VEC3, 0, "POSITION"},  
		{1, 0, GraphicsAPI::VertexType::VEC3, 0, "NORMAL"},
		{2, 0, GraphicsAPI::VertexType::VEC2, 0, "TEXCOORD"} 
	};
													// stride VVVV ie. sizeof(Vertex)
	pipelineCI.vertexInputState.bindings = { {0, 0, 8 * sizeof(float)} };
	pipelineCI.inputAssemblyState = { GraphicsAPI::PrimitiveTopology::TRIANGLE_LIST, false };
	pipelineCI.rasterisationState = { false, false, GraphicsAPI::PolygonMode::FILL, GraphicsAPI::CullMode::BACK, GraphicsAPI::FrontFace::COUNTER_CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f };
	pipelineCI.multisampleState = { 1, false, 1.0f, 0xFFFFFFFF, false, false };
	pipelineCI.depthStencilState = { true, true, GraphicsAPI::CompareOp::LESS_OR_EQUAL, false, false, {}, {}, 0.0f, 1.0f };
	pipelineCI.colorBlendState = { false, GraphicsAPI::LogicOp::NO_OP, {{true, GraphicsAPI::BlendFactor::SRC_ALPHA, GraphicsAPI::BlendFactor::ONE_MINUS_SRC_ALPHA, GraphicsAPI::BlendOp::ADD, GraphicsAPI::BlendFactor::ONE, GraphicsAPI::BlendFactor::ZERO, GraphicsAPI::BlendOp::ADD, (GraphicsAPI::ColorComponentBit)15}}, {0.0f, 0.0f, 0.0f, 0.0f} };
	pipelineCI.colorFormats = { m_colorSwapchainInfos[0].swapchainFormat };
	pipelineCI.depthFormat = m_depthSwapchainInfos[0].swapchainFormat;
	pipelineCI.layout = { {0, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
						 {1, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX} };
	
	m_pipeline = m_graphicsAPI->CreatePipeline(pipelineCI);

	Assimp::Importer imp;
	

	// create models
	const aiScene *scene = imp.ReadFile("data/box.glb", aiProcess_Triangulate | aiProcess_FlipUVs);

	auto mesh = scene->mMeshes[0];

	struct Vertex {
		float x;
		float y;
		float z;
		
		float nx;
		float ny;
		float nz;

		float u;
		float v;
	};

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		vertices.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0.0f, 0.0f });
	}

	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}

	m.vertex_buffer = (VkBuffer*)m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::VERTEX, sizeof(float) * 8, sizeof(Vertex) * vertices.size(), vertices.data() });
	m.index_buffer = (VkBuffer*)m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::INDEX, sizeof(uint32_t), sizeof(uint32_t) * indices.size(), indices.data() });
	m.index_count = indices.size();

	// add debug names
	m_graphicsAPI->SetDebugName("vertex_buffer", (void*)m.vertex_buffer);
}

void GameApp::destroy_resources()
{
	m_graphicsAPI->DestroyPipeline(m_pipeline);
	m_graphicsAPI->DestroyShader(m_fragmentShader);
	m_graphicsAPI->DestroyShader(m_vertexShader);
	m_graphicsAPI->DestroyBuffer(m_uniformBuffer_Camera);
	m_graphicsAPI->DestroyBuffer(m_uniformBuffer_Normals);
}