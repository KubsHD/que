#include "game_app.h"

#include <glm/ext.hpp>

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

void GameApp::render_cube(XrPosef pose, XrVector3f scale, XrVector3f color)
{
	XrMatrix4x4f_CreateTranslationRotationScale(&cameraConstants.model, &pose.position, &pose.orientation, &scale);

	XrMatrix4x4f_Multiply(&cameraConstants.modelViewProj, &cameraConstants.viewProj, &cameraConstants.model);
	cameraConstants.color = { color.x, color.y, color.z, 1.0 };
	size_t offsetCameraUB = sizeof(CameraConstants) * renderCuboidIndex;

	m_graphicsAPI->SetPipeline(m_pipeline);

	m_graphicsAPI->SetBufferData(m_uniformBuffer_Camera, offsetCameraUB, sizeof(CameraConstants), &cameraConstants);
	m_graphicsAPI->SetDescriptor({ 0, m_uniformBuffer_Camera, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, offsetCameraUB, sizeof(CameraConstants) });
	m_graphicsAPI->SetDescriptor({ 1, m_uniformBuffer_Normals, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, 0, sizeof(normals) });

	m_graphicsAPI->UpdateDescriptors();

	m_graphicsAPI->SetVertexBuffers(&m_vertexBuffer, 1);
	m_graphicsAPI->SetIndexBuffer(m_indexBuffer);
	m_graphicsAPI->DrawIndexed(36);

	renderCuboidIndex++;
}

void GameApp::render(FrameRenderInfo& info)
{
	GraphicsAPI::Viewport viewport = { 0.0f, 0.0f, (float)info.width, (float)info.height, 0.0f, 1.0f };
	GraphicsAPI::Rect2D scissor = { {(int32_t)0, (int32_t)0}, {info.width,info.height} };
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

	renderCuboidIndex = 0;
	// Draw a floor. Scale it by 2 in the X and Z, and 0.1 in the Y,
	render_cube({ {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM, 0.0f} }, { 2.0f, 0.1f, 2.0f }, { 0.4f, 0.5f, 0.5f });
	// Draw a "table".
	render_cube({ {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM + 0.9f, -0.7f} }, { 1.0f, 0.2f, 1.0f }, { 0.6f, 0.6f, 0.4f });

	m_graphicsAPI->EndRendering();

	
}

void GameApp::destroy()
{
}


void GameApp::create_resources()
{
	// Vertices for a 1x1x1 meter cube. (Left/Right, Top/Bottom, Front/Back)
	constexpr XrVector4f vertexPositions[] = {
		{+0.5f, +0.5f, +0.5f, 1.0f},
		{+0.5f, +0.5f, -0.5f, 1.0f},
		{+0.5f, -0.5f, +0.5f, 1.0f},
		{+0.5f, -0.5f, -0.5f, 1.0f},
		{-0.5f, +0.5f, +0.5f, 1.0f},
		{-0.5f, +0.5f, -0.5f, 1.0f},
		{-0.5f, -0.5f, +0.5f, 1.0f},
		{-0.5f, -0.5f, -0.5f, 1.0f} };

#define CUBE_FACE(V1, V2, V3, V4, V5, V6) vertexPositions[V1], vertexPositions[V2], vertexPositions[V3], vertexPositions[V4], vertexPositions[V5], vertexPositions[V6],

	XrVector4f cubeVertices[] = {
		CUBE_FACE(2, 1, 0, 2, 3, 1)  // -X
		CUBE_FACE(6, 4, 5, 6, 5, 7)  // +X
		CUBE_FACE(0, 1, 5, 0, 5, 4)  // -Y
		CUBE_FACE(2, 6, 7, 2, 7, 3)  // +Y
		CUBE_FACE(0, 4, 6, 0, 6, 2)  // -Z
		CUBE_FACE(1, 3, 7, 1, 7, 5)  // +Z
	};

	uint32_t cubeIndices[36] = {
		0, 1, 2, 3, 4, 5,        // -X
		6, 7, 8, 9, 10, 11,      // +X
		12, 13, 14, 15, 16, 17,  // -Y
		18, 19, 20, 21, 22, 23,  // +Y
		24, 25, 26, 27, 28, 29,  // -Z
		30, 31, 32, 33, 34, 35,  // +Z
	};

	if (m_apiType == VULKAN) {
		std::vector<char> vertexSource = m_asset_manager->read_all_bytes("data/VertexShader.spv");
		m_vertexShader = m_graphicsAPI->CreateShader({ GraphicsAPI::ShaderCreateInfo::Type::VERTEX, vertexSource.data(), vertexSource.size() });

		std::vector<char> fragmentSource = m_asset_manager->read_all_bytes("data/PixelShader.spv");
		m_fragmentShader = m_graphicsAPI->CreateShader({ GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT, fragmentSource.data(), fragmentSource.size() });
	}

	m_vertexBuffer = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::VERTEX, sizeof(float) * 4, sizeof(cubeVertices), &cubeVertices });

	m_indexBuffer = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::INDEX, sizeof(uint32_t), sizeof(cubeIndices), &cubeIndices });

	size_t numberOfCuboids = 2;
	m_uniformBuffer_Camera = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(CameraConstants) * numberOfCuboids, nullptr });
	m_uniformBuffer_Normals = m_graphicsAPI->CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(normals), &normals });

	GraphicsAPI::PipelineCreateInfo pipelineCI;

	pipelineCI.shaders = { m_vertexShader, m_fragmentShader };
	pipelineCI.vertexInputState.attributes = { {0, 0, GraphicsAPI::VertexType::VEC4, 0, "TEXCOORD"} };
	pipelineCI.vertexInputState.bindings = { {0, 0, 4 * sizeof(float)} };
	pipelineCI.inputAssemblyState = { GraphicsAPI::PrimitiveTopology::TRIANGLE_LIST, false };
	pipelineCI.rasterisationState = { false, false, GraphicsAPI::PolygonMode::FILL, GraphicsAPI::CullMode::BACK, GraphicsAPI::FrontFace::COUNTER_CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f };
	pipelineCI.multisampleState = { 1, false, 1.0f, 0xFFFFFFFF, false, false };
	pipelineCI.depthStencilState = { true, true, GraphicsAPI::CompareOp::LESS_OR_EQUAL, false, false, {}, {}, 0.0f, 1.0f };
	pipelineCI.colorBlendState = { false, GraphicsAPI::LogicOp::NO_OP, {{true, GraphicsAPI::BlendFactor::SRC_ALPHA, GraphicsAPI::BlendFactor::ONE_MINUS_SRC_ALPHA, GraphicsAPI::BlendOp::ADD, GraphicsAPI::BlendFactor::ONE, GraphicsAPI::BlendFactor::ZERO, GraphicsAPI::BlendOp::ADD, (GraphicsAPI::ColorComponentBit)15}}, {0.0f, 0.0f, 0.0f, 0.0f} };
	pipelineCI.colorFormats = { m_colorSwapchainInfos[0].swapchainFormat };
	pipelineCI.depthFormat = m_depthSwapchainInfos[0].swapchainFormat;
	pipelineCI.layout = { {0, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
						 {1, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
						 {2, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT} };
	m_pipeline = m_graphicsAPI->CreatePipeline(pipelineCI);
}

void GameApp::destroy_resources()
{
	m_graphicsAPI->DestroyPipeline(m_pipeline);
	m_graphicsAPI->DestroyShader(m_fragmentShader);
	m_graphicsAPI->DestroyShader(m_vertexShader);
	m_graphicsAPI->DestroyBuffer(m_uniformBuffer_Camera);
	m_graphicsAPI->DestroyBuffer(m_uniformBuffer_Normals);
	m_graphicsAPI->DestroyBuffer(m_indexBuffer);
	m_graphicsAPI->DestroyBuffer(m_vertexBuffer);
}