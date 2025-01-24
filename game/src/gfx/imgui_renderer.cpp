#include "pch.h"

#include "imgui_renderer.h"

#include <lib/imgui/imgui.h>
#include "rhi/vk_descriptor.h"
#include "renderer2.h"
#include <common/vk_initializers.h>

struct ImguiDrawData {
	Vec2 translation;
	Vec2 scale;
};

struct ImguiVertex {
	Vec2 pos;
	Vec2 uv;
	unsigned int color;
};

void ImguiRenderer::init(Renderer2* r2)
{
	{
		DescriptorLayoutBuilder builder;

		builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		builder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		m_imgui_set_layout = builder.build(GfxDevice::device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		m_imgui_set = r2->global_descriptor_allocator.allocate(GfxDevice::device, m_imgui_set_layout);
	}

	{
		VkPipelineLayoutCreateInfo layout_info = vkinit::pipeline_layout_create_info();

		layout_info.pSetLayouts = &m_imgui_set_layout;
		layout_info.setLayoutCount = 1;

		VkPushConstantRange push_constant_range = {};
		push_constant_range.size = sizeof(ImguiDrawData);
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		layout_info.pPushConstantRanges = &push_constant_range;
		layout_info.pushConstantRangeCount = 1;

		vkCreatePipelineLayout(GfxDevice::device, &layout_info, nullptr, &m_imgui_pipeline.layout);

		PipelineBuilder builder;

		auto vs = PipelineBuilder::load_shader_module("shader/debug/imgui.vs_c", GfxDevice::device);
		auto ps = PipelineBuilder::load_shader_module("shader/debug/imgui.ps_c", GfxDevice::device);

		builder.set_shaders(vs, ps);
		builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
		builder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		builder.set_multisampling_none();
		builder.disable_blending();
		builder.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);

		//connect the image format we will draw into, from draw image
		builder.set_depth_format(VK_FORMAT_D32_SFLOAT);

		builder.rasterizer.depthBiasEnable = VK_TRUE;


		builder.pipeline_layout = m_imgui_pipeline.layout;

		m_imgui_pipeline.pipeline = builder.build_pipeline(GfxDevice::device);
	}

	r2->main_deletion_queue.push_function([&]() {
		vkDestroyPipeline(GfxDevice::device, m_imgui_pipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(GfxDevice::device, m_imgui_pipeline.layout, nullptr);
		vkDestroyDescriptorSetLayout(GfxDevice::device, m_imgui_set_layout, nullptr);
	});
}

void ImguiRenderer::render(VkCommandBuffer cmd)
{
	const auto& dd = ImGui::GetDrawData();

	if (dd->CmdListsCount == 0)
		return;



}

