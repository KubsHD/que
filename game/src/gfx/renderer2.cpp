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
#include <game/components/player_component.h>
#include <core/profiler.h>
#include "sky/sky.h"

#include <NGFX_Injection.h>

#include "debug_renderer.h"

#include <tracy/TracyVulkan.hpp>
#include "camera.h"


Renderer2::Renderer2(RenderTarget rt_info, entt::registry& reg) : m_reg(reg)
{
	init_internal(rt_info, reg);
}

void Renderer2::init_internal(RenderTarget rt_info, entt::registry & reg)
{
	QUE_PROFILE;

	this->color_format = VK_FORMAT_R16G16B16A16_SFLOAT;

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

	QUE_INIT_GPU_PROFILER(GfxDevice::physical_device, GfxDevice::device, m_queue, frame.main_command_buffer);


	offscren_color = GfxDevice::create_image(VkExtent2D{ (uint32_t)rt_info.size.x, (uint32_t)rt_info.size.y }, color_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	offscreen_tonemapped = GfxDevice::create_image(VkExtent2D{ (uint32_t)rt_info.size.x, (uint32_t)rt_info.size.y }, rt_info.format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	depth_image = GfxDevice::create_image(VkExtent2D{(uint32_t)rt_info.size.x, (uint32_t)rt_info.size.y},VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depth_prepass = GfxDevice::create_image(VkExtent2D{ (uint32_t)rt_info.size.x, (uint32_t)rt_info.size.y }, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);


	GfxDevice::set_debug_name(depth_prepass.image, "Depth prepass image");
	GfxDevice::set_debug_name(depth_prepass.view, "Depth prepass view");

	GfxDevice::set_debug_name(offscren_color.image, "Offscreen color image");
	GfxDevice::set_debug_name(offscren_color.view, "Offscreen color view");

	GfxDevice::set_debug_name(depth_image.image, "Depth image");
	GfxDevice::set_debug_name(depth_image.view, "Depth view");

	m_shadow_renderer.create(*this);


	create_default_textures();
	create_global_descriptors();

	create_pipelines();

	debug = new DebugRenderer();

	debug->init(this);

	imgui_renderer = new ImguiRenderer();
	imgui_renderer->init(this);

	bloom.init(this);

	main_deletion_queue.push_function([&]() {
		GfxDevice::destroy_image(offscren_color);
		GfxDevice::destroy_image(offscreen_tonemapped);
		GfxDevice::destroy_image(depth_image);
		GfxDevice::destroy_image(depth_prepass);
	});
}

Renderer2::Renderer2(Swapchain& swp, entt::registry& reg) : m_reg(reg)
{
	RenderTarget rt;

	rt.size.x = swp.width;
	rt.size.y = swp.height;

	rt.format = swp.swapchainFormat;

	init_internal(rt, reg);
}

Renderer2::~Renderer2()
{
	vkDeviceWaitIdle(GfxDevice::device);

	main_deletion_queue.execute();

	m_shadow_renderer.destroy();
	debug->destroy();
	delete debug;

	delete imgui_renderer;

	bloom.destroy();

	vkDestroyFence(GfxDevice::device, frame.main_fence, nullptr);
	vkDestroySemaphore(GfxDevice::device, frame.swapchain_semaphore, nullptr);
	vkDestroySemaphore(GfxDevice::device, frame.render_semaphore, nullptr);
	vkDestroyCommandPool(GfxDevice::device, frame.command_pool, nullptr);

	QUE_DESTROY_GPU_PROFILER;
}

void Renderer2::update()
{
	m_shadow_renderer.update();


	auto lights = m_reg.view<core_transform_component, core_uuid_component, core_light_component>();

	for (const auto&& [e, tc, uc, lc] : lights.each())
	{
		if (lc.type == LightType::Spot)
		{
			glm::vec3 direction = glm::vec3(0, -lc.range, 0);
			direction = glm::rotate(glm::quat(tc.rotation.w, tc.rotation.x, tc.rotation.y, tc.rotation.z), direction);

			m_spot_lights[uc.uuid] = gfx::SpotLight{ tc.position, direction, lc.color, lc.intensity, lc.range, lc.angle };
		}


		if (lc.type == LightType::Point)
		{
			m_point_lights[uc.uuid] = gfx::PointLight{ tc.position, lc.color, lc.intensity, lc.range };
		}
	}
}

void Renderer2::load_default_resources()
{
	sky.create(*this, "pure_sky.sky_c", "cube.gltf");

	main_deletion_queue.push_function([&]() {
		sky.clear(GfxDevice::device);
	});

}

void Renderer2::wait_for_frame()
{
	VULKAN_CHECK_NOMSG(vkWaitForFences(GfxDevice::device, 1, &frame.main_fence, true, UINT64_MAX), "Failed to wait for Fence");
	VULKAN_CHECK_NOMSG(vkResetFences(GfxDevice::device, 1, &frame.main_fence), "Failed to reset Fence.")
}

int _frameNumber = 0;

void Renderer2::register_mesh(MeshComponent* mc)
{
	m_reg.emplace<core_mesh_component>(mc->entity->internal_entity, mc->get_model());
}

void Renderer2::set_camera_position(glm::vec3 pos)
{
	m_camera_position = pos;
}

void Renderer2::unregister_mesh(MeshComponent* param1)
{
	m_reg.remove<core_mesh_component>(param1->entity->internal_entity);
}

Vec3 Renderer2::get_camera_position()
{
	return m_camera_position;
}

static uint32_t last_image_index;

void Renderer2::present(VkSwapchainKHR swp)
{
	const VkSemaphore waitSemaphores[] = { frame.render_semaphore };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swp;
	
	presentInfo.pImageIndices = &last_image_index;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = waitSemaphores;

	VULKAN_CHECK_NOMSG(vkQueuePresentKHR(m_queue, &presentInfo));
}


int Renderer2::acquire_image(vkb::Swapchain& swapchain)
{
	VkResult result = vkAcquireNextImageKHR(
		GfxDevice::device,
		swapchain.swapchain,
		1000000000,
		frame.swapchain_semaphore,
		VK_NULL_HANDLE,
		&last_image_index
	);

	return last_image_index;
}

void Renderer2::draw(RenderTarget& rt, CameraRenderData view)
{
	QUE_PROFILE;

	VkCommandBuffer cmd = frame.main_command_buffer;
	VULKAN_CHECK_NOMSG(vkResetCommandBuffer(cmd, 0));

	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VULKAN_CHECK_NOMSG(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	vkutil::transition_image(cmd, rt.image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	draw_internal(cmd, rt, view);

	vkutil::transition_image(cmd, rt.image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	QUE_GPU_COLLECT(cmd);

	VULKAN_CHECK_NOMSG(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdSubmitInfo = vkinit::command_buffer_submit_info(frame.main_command_buffer);

	auto waitInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame.swapchain_semaphore);
	auto signalInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame.render_semaphore);


	VkSubmitInfo2 submitInfo = vkinit::submit_info(&cmdSubmitInfo, &signalInfo, &waitInfo);

	VULKAN_CHECK_NOMSG(vkQueueSubmit2(m_queue, 1, &submitInfo, frame.main_fence));
	_frameNumber++;

}

void Renderer2::draw_xr(RenderTarget rt, CameraRenderData crd)
{
	QUE_PROFILE;

	wait_for_frame();

	VkCommandBuffer cmd = frame.main_command_buffer;
	VULKAN_CHECK_NOMSG(vkResetCommandBuffer(cmd, 0));

	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VULKAN_CHECK_NOMSG(vkBeginCommandBuffer(cmd, &cmdBeginInfo));


	draw_internal(cmd, rt, crd);

	QUE_GPU_COLLECT(cmd);

	VULKAN_CHECK_NOMSG(vkEndCommandBuffer(cmd));


	VkCommandBufferSubmitInfo cmdSubmitInfo = vkinit::command_buffer_submit_info(frame.main_command_buffer);

	VkSubmitInfo2 submitInfo = vkinit::submit_info(&cmdSubmitInfo, nullptr, nullptr);

	VULKAN_CHECK_NOMSG(vkQueueSubmit2(m_queue, 1, &submitInfo, frame.main_fence));
	_frameNumber++;
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

void Renderer2::draw_internal(VkCommandBuffer cmd, RenderTarget rt, CameraRenderData crd)
{
	m_scene_data_cpu.viewProj = crd.projection * crd.view;
	m_scene_data_cpu.view = crd.view;
	m_scene_data_cpu.proj = crd.projection;

	m_scene_data_cpu.camPos = glm::vec3(m_camera_position) + crd.position;

	// fill lights
	m_scene_data_cpu.pointLightCount = m_point_lights.size();
	m_scene_data_cpu.spotLightCount = m_spot_lights.size();


	int i = 0;
	for (auto& [uuid, light] : m_point_lights)
	{
		m_scene_data_cpu.pointLight[i] = light;
		i++;
	}

	i = 0;
	for (auto& [uuid, light] : m_spot_lights)
	{
		m_scene_data_cpu.spotLight[i] = light;
		i++;
	}


	VkClearValue clearValue;
	float flash = std::abs(std::sin(_frameNumber / 240.f));
	clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

	VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(offscren_color.view, &clearValue, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depthAttachment = vkinit::depth_attachment_info(depth_image.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	VkExtent2D _windowExtent = { rt.size.x, rt.size.y };
	VkRenderingInfo render_info = vkinit::rendering_info(_windowExtent, &colorAttachment, &depthAttachment);

	m_shadow_renderer.render(cmd, m_reg, m_scene_data_cpu.camPos);

	m_scene_data_cpu.lightMtx = m_shadow_renderer.light_mtx;

	GfxDevice::upload_buffer(m_scene_data_gpu, 0, &m_scene_data_cpu, sizeof(gfx::SceneData));

	//set dynamic viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = rt.size.x;
	viewport.height = rt.size.y;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = rt.size.x;
	scissor.extent.height = rt.size.y;

	// main pass
	vkCmdBeginRendering(cmd, &render_info);

	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	{
		QUE_GPU_ZONE(cmd, "Render");


		{
			DescriptorWriter writer;
			writer.write_buffer(0, m_scene_data_gpu.buffer, sizeof(gfx::SceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			writer.write_image(1, m_shadow_renderer.directional_shadow_map.view, m_shadow_renderer.shadow_map_sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
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

	}

	vkCmdEndRendering(cmd);
	// main pass end


	// bloom pass
	vkutil::transition_image(cmd, offscreen_tonemapped.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	vkutil::transition_image(cmd, offscren_color.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

	bloom.render(cmd, offscren_color, offscreen_tonemapped);

	vkutil::transition_image(cmd, offscren_color.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	vkutil::transition_image(cmd, offscreen_tonemapped.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	// bloom pass end



	// offscreen to swapchain 
	vkutil::transition_image(cmd, offscren_color.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	vkutil::transition_image(cmd, rt.image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	vkutil::copy_image_to_image(cmd, offscreen_tonemapped.image, rt.image.image, _windowExtent, _windowExtent);
	vkutil::transition_image(cmd, rt.image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	// offscreen to swapchain  end

	// debug pass
	colorAttachment = vkinit::attachment_info(rt.image.view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	depthAttachment = vkinit::depth_attachment_info(depth_image.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

	render_info = vkinit::rendering_info(_windowExtent, &colorAttachment, &depthAttachment);

	vkCmdBeginRendering(cmd, &render_info);

	sky.draw(*this, cmd);

	if (g_engine.config.r_show_debug)
		debug->render(cmd, render_info);

	if (g_engine.config.r_show_imgui)
		imgui_renderer->render(cmd);

	vkCmdEndRendering(cmd);
	// debug pass end

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
		mat_lit.clear(GfxDevice::device);
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
	uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));

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
	 
	texture_white = GfxDevice::create_image(&white, VkExtent2D{ 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	GfxDevice::set_debug_name(texture_white.image, "White Texture");


	// samplers
	VkSamplerCreateInfo sampl = vkinit::sampler_create_info(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, 16);

	sampl.magFilter = VK_FILTER_NEAREST;
	sampl.minFilter = VK_FILTER_NEAREST;
	
	vkCreateSampler(GfxDevice::device, &sampl, nullptr, &default_sampler_nearest);

	sampl.magFilter = VK_FILTER_LINEAR;
	sampl.minFilter = VK_FILTER_LINEAR;
	vkCreateSampler(GfxDevice::device, &sampl, nullptr, &default_sampler_linear);

	sampl.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampl.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	vkCreateSampler(GfxDevice::device, &sampl, nullptr, &default_sampler_linear_clamp);

	main_deletion_queue.push_function([&]() {
		vkDestroySampler(GfxDevice::device, default_sampler_nearest, nullptr);
		vkDestroySampler(GfxDevice::device, default_sampler_linear, nullptr);
		vkDestroySampler(GfxDevice::device, default_sampler_linear_clamp, nullptr);

		GfxDevice::destroy_image(texture_checker);
		GfxDevice::destroy_image(texture_black);
		GfxDevice::destroy_image(texture_normal);
		GfxDevice::destroy_image(texture_white);
	});
}
