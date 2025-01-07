#include "pch.h"

#include "debug_renderer.h"
#include "renderer2.h"
#include <common/vk_initializers.h>

#define MAX_VTX 1000000

struct Im3dVertex
{
	float x;
	float y;
	float z;
	float u;

	float r;
	float g;
	float b;
	float a;
};

struct DrawData {
	VkBuffer buffer;
	glm::mat4 viewProj;
	glm::vec2 viewport;
};

void DebugRenderer::init(Renderer2* r2)
{
	m_r2 = r2;

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


		VULKAN_CHECK_NOMSG(vkCreatePipelineLayout(GfxDevice::device, &pipeline_layout_info, nullptr, &m_triangles_pipeline.layout));

		//use the triangle layout we created
		builder.pipeline_layout = m_triangles_pipeline.layout;
		builder.set_shaders(vs, ps);
		builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
		builder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		builder.set_multisampling_none();
		builder.disable_blending();
		builder.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);

		//connect the image format we will draw into, from draw image
		builder.set_depth_format(VK_FORMAT_D16_UNORM);

		builder.rasterizer.depthBiasEnable = VK_TRUE;

		//finally build the pipeline
		m_triangles_pipeline.pipeline = builder.build_pipeline(GfxDevice::device);

		//clean structures
		vkDestroyShaderModule(GfxDevice::device, vs, nullptr);
		vkDestroyShaderModule(GfxDevice::device, ps, nullptr);
	}
}

void DebugRenderer::begin_frame()
{
	Im3d::AppData& ad = Im3d::GetAppData();

	ad.m_deltaTime = 1.0f / 60.0f;
	ad.m_viewportSize = Im3d::Vec2(m_r2->depth_image.size.width, m_r2->depth_image.size.height);
	ad.m_viewOrigin = Im3d::Vec3(m_r2->get_camera_position().x, m_r2->get_camera_position().y, m_r2->get_camera_position().z);
	ad.m_viewDirection = Im3d::Vec3(0.0f, 0.0f, -1.0f);
	ad.m_worldUp = Im3d::Vec3(0.0f, 1.0f, 0.0f);
	ad.m_projOrtho = false;

	Im3d::NewFrame();
}

void DebugRenderer::render(VkCommandBuffer cmd)
{
	if (Im3d::GetDrawListCount() == 0) {
		return;
	}

	const auto* draw_lists = Im3d::GetDrawLists();
	const auto draw_list_count = Im3d::GetDrawListCount();

	for (int i = 0; i < draw_list_count; i++)
	{
		const Im3d::DrawList& draw_list = draw_lists[i];

		switch (draw_lists->m_primType)
		{
		case Im3d::DrawPrimitive_Triangles:
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_triangles_pipeline.pipeline);
			vkCmdPushConstants(cmd, m_triangles_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DrawData), &draw_list.m_vertexData);
			vkCmdDraw(cmd, draw_list.m_vertexCount, 1, 0, 0);

			break;
		}

		}

	}

}

void DebugRenderer::end_frame()
{
	Im3d::EndFrame();
}

void DebugRenderer::draw_box(Vec3 pos, Vec3 scale, Vec4 color /*= Colors::default_color*/)
{

}
