#include "game_app.h"

#include <asset/mesh.h>
#include <core/asset.h>


#include <glm/ext.hpp>
#include <lib/tiny_gltf.h>

#include <lib/stb_image.h>

#include <vulkan/vulkan.h>
#include <common/GraphicsAPI.h>

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

Mesh m;
GraphicsAPI::Image skybox_image;

VkSampler sampler;

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
	m_graphicsAPI->SetDescriptor({ 0, m_uniformBuffer_Camera, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX, false, offsetCameraUB, sizeof(CameraConstants) });
	m_graphicsAPI->SetDescriptor({ 1, skybox_image.view, sampler, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT, false });

	m_graphicsAPI->UpdateDescriptors();

	m_graphicsAPI->SetVertexBuffers(&m.vertex_buffer, 1);
	m_graphicsAPI->SetIndexBuffer(m.index_buffer);
	m_graphicsAPI->DrawIndexed(m.index_count);
}

static float posx = 0;

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


	renderCuboidIndex = 0;
	// Draw a floor. Scale it by 2 in the X and Z, and 0.1 in the Y,
	render_mesh({ posx, -1.0f, 0.0f}, { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, m);
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
	pipelineCI.rasterisationState = { false, false, GraphicsAPI::PolygonMode::FILL, 
									GraphicsAPI::CullMode::FRONT, GraphicsAPI::FrontFace::COUNTER_CLOCKWISE,
									false, 0.0f, 0.0f, 0.0f, 1.0f };
	pipelineCI.multisampleState = { 1, false, 1.0f, 0xFFFFFFFF, false, false };
	pipelineCI.depthStencilState = { true, true, GraphicsAPI::CompareOp::LESS_OR_EQUAL, false, false, {}, {}, 0.0f, 1.0f };
	pipelineCI.colorBlendState = { false, GraphicsAPI::LogicOp::NO_OP, {{true, GraphicsAPI::BlendFactor::SRC_ALPHA, GraphicsAPI::BlendFactor::ONE_MINUS_SRC_ALPHA, GraphicsAPI::BlendOp::ADD, GraphicsAPI::BlendFactor::ONE, GraphicsAPI::BlendFactor::ZERO, GraphicsAPI::BlendOp::ADD, (GraphicsAPI::ColorComponentBit)15}}, {0.0f, 0.0f, 0.0f, 0.0f} };
	pipelineCI.colorFormats = { m_colorSwapchainInfos[0].swapchainFormat };
	pipelineCI.depthFormat = m_depthSwapchainInfos[0].swapchainFormat;
	pipelineCI.layout = { {0, nullptr, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
						  {1, nullptr, nullptr, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT} };
	
	m_pipeline = m_graphicsAPI->CreatePipeline(pipelineCI);

	m = Asset::load_mesh(*m_graphicsAPI, "data/Avocado.gltf");


	

	auto device = m_graphicsAPI.get()->GetDevice();
	auto allocator = m_graphicsAPI->GetAllocator();

	// load skybox

	int texWidth, texHeight, texChannels;

	float* pixels = stbi_loadf("data/Avocado_baseColor.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels)
		DEBUG_BREAK;



	VkDeviceSize imageSize = texWidth * texHeight * 4;

	GraphicsAPI::Buffer stagingBuffer;

	VkBufferCreateInfo stagingBufferInfo = {};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.pNext = nullptr;

	stagingBufferInfo.size = imageSize;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	//let the VMA library know that this data should be on CPU RAM
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	
	auto result = vmaCreateBuffer(*allocator, &stagingBufferInfo, &vmaallocInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, nullptr);

	void* pixel_ptr = pixels;

	void* data;
	vmaMapMemory(*allocator, stagingBuffer.allocation, &data);

	memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));

	vmaUnmapMemory(*allocator, stagingBuffer.allocation);

	stbi_image_free(pixels);

	VkExtent3D imageExtent;
	imageExtent.width = static_cast<uint32_t>(texWidth);
	imageExtent.height = static_cast<uint32_t>(texHeight);
	imageExtent.depth = 1;

	// create image
	VkImageCreateInfo info = { };
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;

	info.imageType = VK_IMAGE_TYPE_2D;

	info.format = VK_FORMAT_R8G8B8A8_SRGB;
	info.extent = imageExtent;

	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.tiling = VK_IMAGE_TILING_LINEAR;
	info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;



	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;


	result = vmaCreateImage(*allocator, &info, &dimg_allocinfo, &skybox_image.image, &skybox_image.allocation, nullptr);

	m_graphicsAPI->immediate_submit([&](VkCommandBuffer cmd) {
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkImageMemoryBarrier imageBarrier_toTransfer = {};
		imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toTransfer.image = skybox_image.image;
		imageBarrier_toTransfer.subresourceRange = range;

		imageBarrier_toTransfer.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;


		//barrier the image into the transfer-receive layout
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = imageExtent;

		vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer, skybox_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copyRegion);

		VkImageMemoryBarrier imgBarrier_toShaderReadable = imageBarrier_toTransfer;
		imgBarrier_toShaderReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imgBarrier_toShaderReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		imageBarrier_toTransfer.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imgBarrier_toShaderReadable);
	});


	VkImageViewCreateInfo ivinfo = {};
	ivinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivinfo.pNext = nullptr;

	ivinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivinfo.image = skybox_image.image;
	ivinfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	ivinfo.subresourceRange.baseMipLevel = 0;
	ivinfo.subresourceRange.levelCount = 1;
	ivinfo.subresourceRange.baseArrayLayer = 0;
	ivinfo.subresourceRange.layerCount = 1;
	ivinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkCreateImageView(*device, &ivinfo, nullptr, &skybox_image.view);

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
}


void GameApp::destroy_resources()
{
	m_graphicsAPI->DestroyPipeline(m_pipeline);
	m_graphicsAPI->DestroyShader(m_fragmentShader);
	m_graphicsAPI->DestroyShader(m_vertexShader);
	m_graphicsAPI->DestroyBuffer(m_uniformBuffer_Camera);
}