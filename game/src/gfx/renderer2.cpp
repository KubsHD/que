#include "pch.h"

#include "renderer2.h"
#include "rhi/vk_helper.h"
#include "rhi/gfx_device.h"
#include "rhi/gfx_swapchain.h"
#include <common/vk_initializers.h>
#include <common/vk_image.h>
#include "pipeline/builder.h"
#include "vertex.h"

struct ColorData {
	glm::vec3 color;
};

Renderer2::Renderer2(VkFormat color_format)
{
	m_color_format = color_format;

    m_queue = GfxDevice::get_queue(vkb::QueueType::graphics);
	m_queue_family = GfxDevice::get_queue_family(vkb::QueueType::graphics);

    VkFenceCreateInfo finfo = vkh::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo seminfo = vkh::sem_create_info();

	VULKAN_CHECK_NOMSG(vkCreateFence(GfxDevice::device, &finfo, nullptr, &frame.main_fence));
	VULKAN_CHECK_NOMSG(vkCreateSemaphore(GfxDevice::device, &seminfo, nullptr, &frame.swapchain_semaphore));
	VULKAN_CHECK_NOMSG(vkCreateSemaphore(GfxDevice::device, &seminfo, nullptr, &frame.render_semaphore));

	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = m_queue_family;
	VULKAN_CHECK_NOMSG(vkCreateCommandPool(GfxDevice::device, &commandPoolInfo, nullptr, &frame.command_pool));

	VkCommandBufferAllocateInfo cmdAllocInfo = {};
	cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocInfo.pNext = nullptr;
	cmdAllocInfo.commandPool = frame.command_pool;
	cmdAllocInfo.commandBufferCount = 1;
	cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VULKAN_CHECK_NOMSG(vkAllocateCommandBuffers(GfxDevice::device, &cmdAllocInfo, &frame.main_command_buffer));

	create_descriptors();

	create_pipelines();

	std::vector<Vertex2> rect_vertices;
	rect_vertices.resize(4);

	rect_vertices[0].x = 0.5;
	rect_vertices[0].y = -0.5;
	rect_vertices[0].z = 0;

	rect_vertices[1].x = 0.5;
	rect_vertices[1].y = 0.5;
	rect_vertices[1].z = 0;

	rect_vertices[2].x = -0.5;
	rect_vertices[2].y = -0.5;
	rect_vertices[2].z = 0;

	rect_vertices[3].x = -0.5;
	rect_vertices[3].y = 0.5;
	rect_vertices[3].z = 0;

	std::vector<uint32_t> rect_indices;
	rect_indices.resize(6);

	rect_indices[0] = 0;
	rect_indices[1] = 1;
	rect_indices[2] = 2;

	rect_indices[3] = 2;
	rect_indices[4] = 1;
	rect_indices[5] = 3;

	test = upload_mesh(rect_indices, rect_vertices);

	main_deletion_queue.push_function([&]() {
		GfxDevice::destroy_buffer(test.index_buffer);
		GfxDevice::destroy_buffer(test.vertex_buffer);
	});
}

Renderer2::~Renderer2()
{
	main_deletion_queue.execute();

	vkDestroyFence(GfxDevice::device, frame.main_fence, nullptr);
	vkDestroySemaphore(GfxDevice::device, frame.swapchain_semaphore, nullptr);
	vkDestroySemaphore(GfxDevice::device, frame.render_semaphore, nullptr);
	vkDestroyCommandPool(GfxDevice::device, frame.command_pool, nullptr);
}

int _frameNumber = 0;

void Renderer2::draw(Swapchain& swp, int image_index)
{
	VULKAN_CHECK_NOMSG(vkWaitForFences(GfxDevice::device, 1, &frame.main_fence, true, UINT64_MAX), "Failed to wait for Fence");
	VULKAN_CHECK_NOMSG(vkResetFences(GfxDevice::device, 1, &frame.main_fence), "Failed to reset Fence.")

	VkCommandBuffer cmd = frame.main_command_buffer;
	VULKAN_CHECK_NOMSG(vkResetCommandBuffer(cmd, 0));


	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	//start the command buffer recording
	VULKAN_CHECK_NOMSG(vkBeginCommandBuffer(cmd, &cmdBeginInfo));


	VkClearValue clearValue;
	float flash = std::abs(std::sin(_frameNumber / 120.f));
	clearValue = { { 0.0f, 0.0f, flash, 1.0f } };
	VkImageSubresourceRange clearRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

	VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(swp.swapchainImages[image_index], &clearValue, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkExtent2D _windowExtent = {swp.width, swp.height };

	VkRenderingInfo render_info = vkinit::rendering_info(_windowExtent, &colorAttachment, nullptr);


	vkCmdBeginRendering(cmd, &render_info);

	//set dynamic viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = swp.width;
	viewport.height = swp.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = swp.width;
	scissor.extent.height = swp.height;

	vkCmdSetScissor(cmd, 0, 1, &scissor);

	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(cmd, 0, 1, &test.vertex_buffer.buffer, offsets);
	vkCmdBindIndexBuffer(cmd, test.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	ColorData cd;
	cd.color = glm::vec3(1, 0, 0);

	vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ColorData), &cd);


	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pip);
	vkCmdDrawIndexed(cmd, test.index_count, 1, 0, 0, 0);

	vkCmdEndRendering(cmd);

	VULKAN_CHECK_NOMSG(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdSubmitInfo = vkinit::command_buffer_submit_info(cmd);
	VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo2 submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

	VULKAN_CHECK_NOMSG(vkQueueSubmit2(m_queue, 1, &submitInfo, frame.main_fence));

	_frameNumber++;
}

GPUMeshBuffer Renderer2::upload_mesh(std::vector<uint32_t> indices, std::vector<Vertex2> vertices)
{
	const size_t vertexBufferSize = vertices.size() * sizeof(Vertex2);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffer meshBuffer;

	meshBuffer.vertex_buffer = GfxDevice::create_buffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	meshBuffer.index_buffer = GfxDevice::create_buffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	meshBuffer.index_count = static_cast<uint32_t>(indices.size());

	GPUBuffer staging = GfxDevice::create_buffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	void* data;
	vmaMapMemory(GfxDevice::allocator, staging.allocation, &data);

	memcpy(data, vertices.data(), vertexBufferSize);
	memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

	vmaUnmapMemory(GfxDevice::allocator, staging.allocation);

	GfxDevice::immediate_submit([&](VkCommandBuffer cmd) {
		VkBufferCopy vertexCopy{ 0 };
		vertexCopy.dstOffset = 0;
		vertexCopy.srcOffset = 0;
		vertexCopy.size = vertexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, meshBuffer.vertex_buffer.buffer, 1, &vertexCopy);

		VkBufferCopy indexCopy{ 0 };
		indexCopy.dstOffset = 0;
		indexCopy.srcOffset = vertexBufferSize;
		indexCopy.size = indexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, meshBuffer.index_buffer.buffer, 1, &indexCopy);
	});

	GfxDevice::destroy_buffer(staging);

	return meshBuffer;
}



void Renderer2::create_pipelines()
{


	PipelineBuilder pipelineBuilder;

	auto vs = PipelineBuilder::load_shader_module("shader/unlit.vs_c", GfxDevice::device);
	auto ps = PipelineBuilder::load_shader_module("shader/unlit.ps_c", GfxDevice::device);

	VkPushConstantRange bufferRange{};
	bufferRange.offset = 0;
	bufferRange.size = sizeof(ColorData);
	bufferRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();
	pipeline_layout_info.pPushConstantRanges = &bufferRange;
	pipeline_layout_info.pushConstantRangeCount = 1;

	VULKAN_CHECK_NOMSG(vkCreatePipelineLayout(GfxDevice::device, &pipeline_layout_info, nullptr, &layout));

	//use the triangle layout we created
	pipelineBuilder.pipeline_layout = layout;
	pipelineBuilder.set_shaders(vs, ps);
	pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.set_multisampling_none();
	pipelineBuilder.disable_blending();
	pipelineBuilder.disable_depthtest();

	//connect the image format we will draw into, from draw image
	pipelineBuilder.set_color_attachment_format(m_color_format);
	pipelineBuilder.set_depth_format(VK_FORMAT_UNDEFINED);


	pipelineBuilder.vertex_input_info.vertexBindingDescriptionCount = 1;
	pipelineBuilder.vertex_input_info.pVertexBindingDescriptions = &Vertex2::get_binding_description();

	pipelineBuilder.vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex2::get_attributes_descriptions().size());
	pipelineBuilder.vertex_input_info.pVertexAttributeDescriptions = Vertex2::get_attributes_descriptions().data();

	//finally build the pipeline
	pip = pipelineBuilder.build_pipeline(GfxDevice::device);

	//clean structures
	vkDestroyShaderModule(GfxDevice::device, vs, nullptr);
	vkDestroyShaderModule(GfxDevice::device, ps, nullptr);

	main_deletion_queue.push_function([&]() {
		vkDestroyPipelineLayout(GfxDevice::device, layout, nullptr);
		vkDestroyPipeline(GfxDevice::device, pip, nullptr);
		});
}

void Renderer2::create_descriptors()
{
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
	{
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
	};

	global_descriptor_allocator.init_pool(GfxDevice::device, 10, sizes);

	{
		DescriptorLayoutBuilder builder;
		builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		unlit_pipeline_desc_layout = builder.build(GfxDevice::device, VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	unlit_pipeline_desc_set = global_descriptor_allocator.allocate(GfxDevice::device, unlit_pipeline_desc_layout);

	main_deletion_queue.push_function([&]() {
		global_descriptor_allocator.destroy_pool(GfxDevice::device);
		vkDestroyDescriptorSetLayout(GfxDevice::device, unlit_pipeline_desc_layout, nullptr);
	});
}
