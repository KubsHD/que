#include "pch.h"

#include "debug_renderer.h"
#include "renderer2.h"
#include <common/vk_initializers.h>
#include <tracy/TracyVulkan.hpp>
#include <core/profiler.h>
#include "rhi/vk_helper.h"

#define MAX_VTX 10000000

struct DrawData
{
	uint32_t offset;
	glm::mat4 viewProj;
	glm::vec2 viewport;
};

struct test
{
	uint32_t offset;
};

void DebugRenderer::init(Renderer2 *r2)
{
	m_r2 = r2;

	{
		DescriptorLayoutBuilder builder;
		builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		m_im3d_set_layout = builder.build(GfxDevice::device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		m_im3d_set = m_r2->global_descriptor_allocator.allocate(GfxDevice::device, m_im3d_set_layout);
	}

	{
		// triangles
		const auto vs =
			PipelineBuilder::load_shader_module("shader/debug/im3d_triangles.vs_c", GfxDevice::device);
		const auto ps =
			PipelineBuilder::load_shader_module("shader/debug/im3d_triangles.ps_c", GfxDevice::device);

		PipelineBuilder builder;
		builder.set_shaders(vs, ps);

		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

		VkPushConstantRange range{};
		range.offset = 0;
		range.size = sizeof(DrawData);
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		pipeline_layout_info.pPushConstantRanges = &range;
		pipeline_layout_info.pushConstantRangeCount = 1;

		auto sets = {m_im3d_set_layout};

		pipeline_layout_info.pSetLayouts = sets.begin();
		pipeline_layout_info.setLayoutCount = sets.size();

		VULKAN_CHECK_NOMSG(vkCreatePipelineLayout(GfxDevice::device, &pipeline_layout_info, nullptr, &m_triangles_pipeline.layout));

		// use the triangle layout we created
		builder.pipeline_layout = m_triangles_pipeline.layout;
		builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
		builder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		builder.set_multisampling_none();
		builder.disable_blending();
		builder.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);

		// connect the image format we will draw into, from draw image
		builder.set_depth_format(VK_FORMAT_D32_SFLOAT);

		builder.rasterizer.depthBiasEnable = VK_TRUE;

		// finally build the pipeline
		m_triangles_pipeline.pipeline = builder.build_pipeline(GfxDevice::device);

		// clean structures
		vkDestroyShaderModule(GfxDevice::device, vs, nullptr);
		vkDestroyShaderModule(GfxDevice::device, ps, nullptr);
	}

	{
		// lines
		const auto vs =
			PipelineBuilder::load_shader_module("shader/debug/im3d_lines.vs_c", GfxDevice::device);
		const auto ps =
			PipelineBuilder::load_shader_module("shader/debug/im3d_lines.ps_c", GfxDevice::device);
		const auto gs =
			PipelineBuilder::load_shader_module("shader/debug/im3d_lines.gs_c", GfxDevice::device);

		PipelineBuilder builder;
		builder.set_shaders(vs, ps, gs);

		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

		VkPushConstantRange range{};
		range.offset = 0;
		range.size = sizeof(DrawData);
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		pipeline_layout_info.pPushConstantRanges = &range;
		pipeline_layout_info.pushConstantRangeCount = 1;

		auto sets = {m_im3d_set_layout};

		pipeline_layout_info.pSetLayouts = sets.begin();
		pipeline_layout_info.setLayoutCount = sets.size();

		VULKAN_CHECK_NOMSG(vkCreatePipelineLayout(GfxDevice::device, &pipeline_layout_info, nullptr, &m_lines_pipeline.layout));

		// use the triangle layout we created
		builder.pipeline_layout = m_lines_pipeline.layout;
		builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
		builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
		builder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		builder.set_multisampling_none();
		builder.disable_blending();
		builder.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);

		// connect the image format we will draw into, from draw image
		builder.set_depth_format(VK_FORMAT_D32_SFLOAT);
		builder.rasterizer.depthBiasEnable = VK_TRUE;

		// finally build the pipeline
		m_lines_pipeline.pipeline = builder.build_pipeline(GfxDevice::device);

		// clean structures
		vkDestroyShaderModule(GfxDevice::device, vs, nullptr);
		vkDestroyShaderModule(GfxDevice::device, ps, nullptr);
		vkDestroyShaderModule(GfxDevice::device, gs, nullptr);
	}

	{
		// points
		const auto vs =
			PipelineBuilder::load_shader_module("shader/debug/im3d_points.vs_c", GfxDevice::device);
		const auto ps =
			PipelineBuilder::load_shader_module("shader/debug/im3d_points.ps_c", GfxDevice::device);

		PipelineBuilder builder;
		builder.set_shaders(vs, ps);

		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

		VkPushConstantRange range{};
		range.offset = 0;
		range.size = sizeof(DrawData);
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		pipeline_layout_info.pPushConstantRanges = &range;
		pipeline_layout_info.pushConstantRangeCount = 1;

		auto sets = {m_im3d_set_layout};

		pipeline_layout_info.pSetLayouts = sets.begin();
		pipeline_layout_info.setLayoutCount = sets.size();

		VULKAN_CHECK_NOMSG(vkCreatePipelineLayout(GfxDevice::device, &pipeline_layout_info, nullptr, &m_points_pipeline.layout));

		// use the triangle layout we created
		builder.pipeline_layout = m_points_pipeline.layout;
		builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
		builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
		builder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		builder.set_multisampling_none();
		builder.disable_blending();
		builder.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);

		// connect the image format we will draw into, from draw image
		builder.set_depth_format(VK_FORMAT_D32_SFLOAT);
		builder.rasterizer.depthBiasEnable = VK_TRUE;

		// finally build the pipeline
		m_points_pipeline.pipeline = builder.build_pipeline(GfxDevice::device);

		// clean structures
		vkDestroyShaderModule(GfxDevice::device, vs, nullptr);
		vkDestroyShaderModule(GfxDevice::device, ps, nullptr);
	}

	m_im3d_buffer = GfxDevice::create_buffer(sizeof(Im3d::VertexData) * MAX_VTX, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
}

void DebugRenderer::destroy()
{
	GfxDevice::destroy_buffer(m_im3d_buffer);
	vkDestroyPipeline(GfxDevice::device, m_triangles_pipeline.pipeline, nullptr);
	vkDestroyPipeline(GfxDevice::device, m_lines_pipeline.pipeline, nullptr);
	vkDestroyPipeline(GfxDevice::device, m_points_pipeline.pipeline, nullptr);
	vkDestroyPipelineLayout(GfxDevice::device, m_triangles_pipeline.layout, nullptr);
	vkDestroyPipelineLayout(GfxDevice::device, m_lines_pipeline.layout, nullptr);
	vkDestroyPipelineLayout(GfxDevice::device, m_points_pipeline.layout, nullptr);
	vkDestroyDescriptorSetLayout(GfxDevice::device, m_im3d_set_layout, nullptr);
}

void DebugRenderer::begin_frame(DebugRendererRenderInfo *info)
{
	Im3d::AppData &ad = Im3d::GetAppData();

	ad.m_deltaTime = 1.0f / 75.0f;
	ad.m_viewportSize = Im3d::Vec2(m_r2->depth_image.size.width, m_r2->depth_image.size.height);
	ad.m_viewOrigin = Im3d::Vec3(m_r2->m_scene_data_cpu.camPos.x, m_r2->m_scene_data_cpu.camPos.y, m_r2->m_scene_data_cpu.camPos.z);
	ad.m_viewDirection = Im3d::Vec3(0.0f, 0.0f, -1.0f);
	ad.m_worldUp = Im3d::Vec3(0.0f, 1.0f, 0.0f);
	ad.m_projOrtho = false;

	if (info)
	{
		ad.m_worldUp = Im3d::Vec3(info->world_up.x, info->world_up.y, info->world_up.z);
		ad.m_viewDirection = Im3d::Vec3(info->view_direction.x, info->view_direction.y, info->view_direction.z);
	}

	Im3d::NewFrame();
}

void DebugRenderer::render(VkCommandBuffer cmd, VkRenderingInfo &info)
{

	if (Im3d::GetDrawListCount() == 0)
	{
		return;
	}

	QUE_GPU_ZONE(cmd, "Debug");

	fill_buffer();

	const auto *draw_lists = Im3d::GetDrawLists();
	const auto draw_list_count = Im3d::GetDrawListCount();

	// vkCmdBeginRendering(cmd, &info);

	DescriptorWriter writer;
	writer.write_buffer(0, m_im3d_buffer.buffer, sizeof(Im3d::VertexData) * MAX_VTX, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	writer.update_set(GfxDevice::device, m_im3d_set);

	uint32_t offset = 0;

	for (uint32_t i = 0; i < draw_list_count; i++)
	{
		DrawData dd;

		dd.offset = offset;
		dd.viewProj = m_r2->m_scene_data_cpu.viewProj;
		dd.viewport = glm::vec2(info.renderArea.extent.width, info.renderArea.extent.height);

		const Im3d::DrawList &draw_list = draw_lists[i];

		switch (draw_lists->m_primType)
		{
		case Im3d::DrawPrimitive_Triangles:
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_triangles_pipeline.pipeline);

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_triangles_pipeline.layout, 0, 1, &m_im3d_set, 0, nullptr);

			vkCmdPushConstants(cmd, m_triangles_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DrawData), &dd);

			vkCmdDraw(cmd, draw_list.m_vertexCount, 1, 0, 0);
			break;
		}
		case Im3d::DrawPrimitive_Lines:
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_lines_pipeline.pipeline);

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_lines_pipeline.layout, 0, 1, &m_im3d_set, 0, nullptr);

			vkCmdPushConstants(cmd, m_lines_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DrawData), &dd);
			vkCmdDraw(cmd, draw_list.m_vertexCount, 1, 0, 0);
			break;
		}
		case Im3d::DrawPrimitive_Points:
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_points_pipeline.pipeline);

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_points_pipeline.layout, 0, 1, &m_im3d_set, 0, nullptr);

			vkCmdPushConstants(cmd, m_points_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DrawData), &dd);
			vkCmdDraw(cmd, draw_list.m_vertexCount, 1, 0, 0);
			break;
		}
		}

		offset += draw_list.m_vertexCount;
	}

	// vkCmdEndRendering(cmd);
}

void DebugRenderer::end_frame()
{
	Im3d::EndFrame();
}

void DebugRenderer::draw_box(Vec3 pos, Vec3 scale, Vec4 color /*= Colors::default_color*/)
{
}

void DebugRenderer::fill_buffer()
{
	auto *draw_lists = Im3d::GetDrawLists();
	const auto draw_list_count = Im3d::GetDrawListCount();

	size_t offset = 0;

	for (int i = 0; i < draw_list_count; i++)
	{
		const Im3d::DrawList &draw_list = draw_lists[i];

		GfxDevice::upload_buffer(m_im3d_buffer, offset, (void *)draw_list.m_vertexData, draw_list.m_vertexCount * sizeof(Im3d::VertexData));

		offset += sizeof(Im3d::VertexData) * draw_list.m_vertexCount;
	}
}
