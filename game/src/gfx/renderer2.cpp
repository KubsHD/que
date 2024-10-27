#include "pch.h"

#include "renderer2.h"
#include "rhi/vk_helper.h"
#include "rhi/gfx_device.h"
#include "rhi/gfx_swapchain.h"
#include <common/vk_initializers.h>
#include <common/vk_image.h>
#include "pipeline/builder.h"
#include "vertex.h"

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

void Renderer2::draw(VkImage swapchain_image)
{
	VULKAN_CHECK_NOMSG(vkWaitForFences(GfxDevice::device, 1, &frame.main_fence, true, UINT64_MAX), "Failed to wait for Fence");
	VULKAN_CHECK_NOMSG(vkResetFences(GfxDevice::device, 1, &frame.main_fence), "Failed to reset Fence.")

	VkCommandBuffer cmd = frame.main_command_buffer;
	VULKAN_CHECK_NOMSG(vkResetCommandBuffer(cmd, 0));


	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	//start the command buffer recording
	VULKAN_CHECK_NOMSG(vkBeginCommandBuffer(cmd, &cmdBeginInfo));


	VkClearColorValue clearValue;
	float flash = std::abs(std::sin(_frameNumber / 120.f));
	clearValue = { { 0.0f, 0.0f, flash, 1.0f } };
	VkImageSubresourceRange clearRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

	vkinit::transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	vkCmdClearColorImage(cmd, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &clearRange);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pip);

	vkinit::transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


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

struct ColorData {
	glm::vec3 color;
};

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
	pipelineBuilder.vertex_input_info.pVertexBindingDescriptions = &Vertex::get_binding_description();

	pipelineBuilder.vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::get_attributes_descriptions().size());
	pipelineBuilder.vertex_input_info.pVertexAttributeDescriptions = Vertex::get_attributes_descriptions().data();

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
