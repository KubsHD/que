#include "pch.h"
#include "dir_light_pipeline.h"
#include "builder.h"
#include <common/vk_initializers.h>

GPUPipeline pipeline::create_dir_light_pipeline(Renderer2& ren)
{
	GPUPipeline pipeline;
	PipelineBuilder builder;

	auto vs = PipelineBuilder::load_shader_module("shader/dir_light_sm.vs_c", GfxDevice::device);
	auto ps = PipelineBuilder::load_shader_module("shader/dir_light_sm.ps_c", GfxDevice::device);

	builder.set_shaders(vs, ps);

	VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

	VkPushConstantRange range{};
	range.offset = 0;
	range.size = sizeof(GPUDrawPushConstants);
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	pipeline_layout_info.pPushConstantRanges = &range;
	pipeline_layout_info.pushConstantRangeCount = 1;


	VULKAN_CHECK_NOMSG(vkCreatePipelineLayout(GfxDevice::device, &pipeline_layout_info, nullptr, &pipeline.layout));

	//use the triangle layout we created
	builder.pipeline_layout = pipeline.layout;
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
	builder.vertex_input_info.vertexBindingDescriptionCount = 1;

	// referencing this directly crashes vkCreateGraphicsPipelines in release mode
	auto bdata = Vertex2::get_binding_description();
	builder.vertex_input_info.pVertexBindingDescriptions = &bdata;
	builder.vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex2::get_attributes_descriptions().size());

	// referencing this directly crashes vkCreateGraphicsPipelines in release mode
	auto data = Vertex2::get_attributes_descriptions();
	builder.vertex_input_info.pVertexAttributeDescriptions = data.data();

	//finally build the pipeline
	pipeline.pipeline = builder.build_pipeline(GfxDevice::device);

	//clean structures
	vkDestroyShaderModule(GfxDevice::device, vs, nullptr);
	vkDestroyShaderModule(GfxDevice::device, ps, nullptr);

	return pipeline;
}

