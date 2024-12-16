#include "pch.h"

#include "renderer2.h"
#include "rhi/vk_helper.h"
#include "rhi/gfx_device.h"
#include "rhi/gfx_swapchain.h"
#include <common/vk_initializers.h>
#include <gfx/rhi/vk_image.h>
#include "pipeline/builder.h"
#include "vertex.h"
#include <common/glm_helpers.h>
#include <entt/entt.hpp>

#include <core/components/components.h>
#include <game/components/mesh_component.h>
#include <core/profiler.h>
#include "sky/sky.h"

#include <NGFX_Injection.h>

#include "debug_renderer.h"

#include <tracy/TracyVulkan.hpp>

static tracy::VkCtx* ctx;

Renderer2::Renderer2(Swapchain& swapchain_info, entt::registry& reg) : m_reg(reg)
{
	QUE_PROFILE;

	this->color_format = swapchain_info.swapchainFormat;

	depth_format = VK_FORMAT_D32_SFLOAT;

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

	ctx = TracyVkContext(GfxDevice::physical_device, GfxDevice::device, m_queue, frame.main_command_buffer);

	depth_image = GfxDevice::create_image(VkExtent2D{(uint32_t)swapchain_info.width, (uint32_t)swapchain_info.height},VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	m_shadow_renderer.create(*this);

	create_default_textures();
	create_global_descriptors();

	create_pipelines();

	debug = new DebugRenderer();

	debug->init(this);

	main_deletion_queue.push_function([&]() {
		GfxDevice::destroy_image(depth_image);
	});
}

Renderer2::~Renderer2()
{
	main_deletion_queue.execute();

	delete debug;

	m_shadow_renderer.destroy();

	vkDestroyFence(GfxDevice::device, frame.main_fence, nullptr);
	vkDestroySemaphore(GfxDevice::device, frame.swapchain_semaphore, nullptr);
	vkDestroySemaphore(GfxDevice::device, frame.render_semaphore, nullptr);
	vkDestroyCommandPool(GfxDevice::device, frame.command_pool, nullptr);
}

void Renderer2::load_default_resources()
{
	sky.create(*this, "pure_sky.sky_c", "cube.gltf");

	main_deletion_queue.push_function([&]() {
		sky.clear(GfxDevice::device);
	});

}

int _frameNumber = 0;

void Renderer2::register_mesh(const MeshComponent* mc)
{
	m_reg.emplace<core_mesh_component>(mc->entity->internal_entity, mc->get_model());
}

void Renderer2::draw(Swapchain& swp, int image_index, XrView view)
{
	ImGui::NewFrame();

	VULKAN_CHECK_NOMSG(vkWaitForFences(GfxDevice::device, 1, &frame.main_fence, true, UINT64_MAX), "Failed to wait for Fence");
	VULKAN_CHECK_NOMSG(vkResetFences(GfxDevice::device, 1, &frame.main_fence), "Failed to reset Fence.")

	VkCommandBuffer cmd = frame.main_command_buffer;
	VULKAN_CHECK_NOMSG(vkResetCommandBuffer(cmd, 0));

	glm::mat4 projection = glm::to_glm_projection(view.fov);

	// https://gitlab.com/amini-allight/openxr-tutorial/-/blob/master/examples/part-9/openxr_example.cpp?ref_type=heads#L1434
	glm::mat4 viewMatrix = glm::inverse(
		glm::translate(glm::mat4(1.0f), glm::vec3(view.pose.position.x, view.pose.position.y, view.pose.position.z))
		* glm::mat4_cast(glm::quat(view.pose.orientation.w, view.pose.orientation.x, view.pose.orientation.y, view.pose.orientation.z))
	);

	m_scene_data_cpu.viewProj = projection * viewMatrix;
	m_scene_data_cpu.view = viewMatrix;
	m_scene_data_cpu.proj = projection;
	m_scene_data_cpu.camPos = glm::vec4(view.pose.position.x, view.pose.position.y, view.pose.position.z, 1.0f);

	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VULKAN_CHECK_NOMSG(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//TracyVkZone(ctx, cmd, "Render");

	VkClearValue clearValue;
	float flash = std::abs(std::sin(_frameNumber / 120.f));
	clearValue = { { 0.0f, 0.0f, flash, 1.0f } };
	
	VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(swp.swapchainImages[image_index], &clearValue, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depthAttachment = vkinit::depth_attachment_info(depth_image.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	VkExtent2D _windowExtent = {swp.width, swp.height };
	VkRenderingInfo render_info = vkinit::rendering_info(_windowExtent, &colorAttachment, &depthAttachment);

	m_shadow_renderer.render(cmd, m_reg, m_scene_data_cpu.camPos);
	m_shadow_renderer.render_imgui();

	m_scene_data_cpu.lightMtx = m_shadow_renderer.light_mtx;

	GfxDevice::upload_buffer(m_scene_data_gpu, 0, &m_scene_data_cpu, sizeof(gfx::SceneData));

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

	draw_internal(cmd);

	vkCmdEndRendering(cmd);

	//TracyVkCollect(ctx, cmd);

	VULKAN_CHECK_NOMSG(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdSubmitInfo = vkinit::command_buffer_submit_info(cmd);
	VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo2 submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

	VULKAN_CHECK_NOMSG(vkQueueSubmit2(m_queue, 1, &submitInfo, frame.main_fence));

	_frameNumber++;

	ImGui::Render();
}

GPUMeshBuffer Renderer2::upload_mesh(std::vector<uint32_t> indices, std::vector<Vertex2> vertices)
{
	const size_t vertexBufferSize = vertices.size() * sizeof(Vertex2);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffer meshBuffer;

	meshBuffer.vertex_buffer = GfxDevice::create_buffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	meshBuffer.index_buffer = GfxDevice::create_buffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
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

void Renderer2::draw_internal(VkCommandBuffer cmd)
{

	{
		DescriptorWriter writer;
		writer.write_buffer(0, m_scene_data_gpu.buffer, sizeof(gfx::SceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		writer.write_image(1, m_shadow_renderer.shadow_map.view, m_shadow_renderer.shadow_map_sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		writer.update_set(GfxDevice::device, scene_data_set);
	}


	GPUDrawPushConstants pc;
	VkDeviceSize offset = 0;

	auto modelsToRender = m_reg.view<core_transform_component, core_mesh_component>();
	for (const auto&& [e, tc, mc] : modelsToRender.each())
	{
		tc.calculate_matrix();
		glm::mat4 model_matrix = tc.matrix;

		pc.model = model_matrix;

		for (auto& mesh : mc.model->meshes)
		{
			auto minst = mc.model->materials2[mesh.material_index];

			if (minst.pipeline == nullptr)
			{
				continue;
			}

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, minst.pipeline->pipeline);

			vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertex_buffer.buffer, &offset);
			vkCmdBindIndexBuffer(cmd, mesh.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, minst.pipeline->layout, 0, 1, &scene_data_set, 0, nullptr);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, minst.pipeline->layout, 1, 1, &mc.model->materials2[mesh.material_index].material_set, 0, nullptr);

			vkCmdPushConstants(cmd, minst.pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GPUDrawPushConstants), &pc);
			vkCmdDrawIndexed(cmd, mesh.index_count, 1, 0, 0, 0);
		}
	};


	debug->render(cmd);

	sky.draw(*this, cmd);
}

void Renderer2::create_pipelines()
{
	m_scene_data_gpu = GfxDevice::create_buffer(sizeof(gfx::SceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	GfxDevice::upload_buffer(m_scene_data_gpu, 0, &m_scene_data_cpu, sizeof(gfx::SceneData));

	mat_unlit.create(this);
	mat_lit.create(this);


	main_deletion_queue.push_function([&]() {
		GfxDevice::destroy_buffer(m_scene_data_gpu);
		mat_unlit.clear(GfxDevice::device);
	});
}

void Renderer2::create_global_descriptors()
{
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
	{
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 20 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 50 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 50 }
	};

	global_descriptor_allocator.init_pool(GfxDevice::device, 1000, sizes);

	{
		DescriptorLayoutBuilder builder;
		builder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		builder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		scene_data_set_layout = builder.build(GfxDevice::device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	scene_data_set = global_descriptor_allocator.allocate(GfxDevice::device, scene_data_set_layout);
	GfxDevice::set_debug_name(scene_data_set, "Global Scene Descriptor Set");

	main_deletion_queue.push_function([&]() {
		global_descriptor_allocator.destroy_pool(GfxDevice::device);
		vkDestroyDescriptorSetLayout(GfxDevice::device, scene_data_set_layout, nullptr);
	});
}

void Renderer2::create_default_textures()
{


	// https://vkguide.dev/docs/new_chapter_4/textures/
	uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	uint32_t normal = glm::packUnorm4x8(glm::vec4(0.5, 0.5, 1, 1));

	std::array<uint32_t, 16 * 16 > pixels; //for 16x16 checkerboard texture
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
		}
	}
	texture_checker = GfxDevice::create_image(pixels.data(), VkExtent2D{ 16, 16 }, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	GfxDevice::set_debug_name(texture_checker.image, "Checkerboard Texture");
	

	texture_black = GfxDevice::create_image(&black, VkExtent2D{ 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	GfxDevice::set_debug_name(texture_black.image, "Black Texture");

	texture_normal = GfxDevice::create_image(&normal, VkExtent2D{ 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	// samplers
	VkSamplerCreateInfo sampl = vkinit::sampler_create_info(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, 16);

	sampl.magFilter = VK_FILTER_NEAREST;
	sampl.minFilter = VK_FILTER_NEAREST;
	
	vkCreateSampler(GfxDevice::device, &sampl, nullptr, &default_sampler_nearest);

	sampl.magFilter = VK_FILTER_LINEAR;
	sampl.minFilter = VK_FILTER_LINEAR;
	vkCreateSampler(GfxDevice::device, &sampl, nullptr, &default_sampler_linear);

	main_deletion_queue.push_function([&]() {
		vkDestroySampler(GfxDevice::device, default_sampler_nearest, nullptr);
		vkDestroySampler(GfxDevice::device, default_sampler_linear, nullptr);

		GfxDevice::destroy_image(texture_checker);
		GfxDevice::destroy_image(texture_black);
		GfxDevice::destroy_image(texture_normal);
	});
}
