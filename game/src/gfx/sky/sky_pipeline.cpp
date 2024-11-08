#include "pch.h"

#include "sky_pipeline.h"

#include <gfx/mesh.h>
#include <core/profiler.h>
#include <gfx/rhi/vk_descriptor.h>
#include <common/vk_initializers.h>
#include <gfx/renderer2.h>

namespace pipeline
{
	SkyIntermiedatePipeline create_sky_pipeline(Renderer2& ren)
	{
		QUE_PROFILE;

		PipelineBuilder pipelineBuilder;
		SkyIntermiedatePipeline pip{};

		auto vs = PipelineBuilder::load_shader_module("shader/sky.vs_c", GfxDevice::device);
		auto ps = PipelineBuilder::load_shader_module("shader/sky.ps_c", GfxDevice::device);

		VkPushConstantRange range;
		range.offset = 0;
		range.size = sizeof(GPUModelConstant);
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;


		DescriptorLayoutBuilder builder;
		builder.add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		pip.descriptor_set_layout = builder.build(GfxDevice::device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();


		pipeline_layout_info.pPushConstantRanges = &range;
		pipeline_layout_info.pushConstantRangeCount = 1;

		auto sets = { ren.scene_data_set_layout, pip.descriptor_set_layout };
		pipeline_layout_info.setLayoutCount = 1;
		pipeline_layout_info.pSetLayouts = &pip.descriptor_set_layout;

		pip.descriptor_set = ren.global_descriptor_allocator.allocate(GfxDevice::device, pip.descriptor_set_layout);

		VULKAN_CHECK_NOMSG(vkCreatePipelineLayout(GfxDevice::device, &pipeline_layout_info, nullptr, &pip.pipeline.layout));

		//use the triangle layout we created
		pipelineBuilder.pipeline_layout = pip.pipeline.layout;
		pipelineBuilder.set_shaders(vs, ps);
		pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
		pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		pipelineBuilder.set_multisampling_none();
		pipelineBuilder.disable_blending();
		pipelineBuilder.disable_depthtest();
		pipelineBuilder.set_color_attachment_format(ren.color_format);

		// referencing this directly crashes vkCreateGraphicsPipelines in release mode
		auto bdata = Vertex2::get_binding_description();
		pipelineBuilder.vertex_input_info.pVertexBindingDescriptions = &bdata;
		pipelineBuilder.vertex_input_info.vertexBindingDescriptionCount = 1;

		// referencing this directly crashes vkCreateGraphicsPipelines in release mode
		auto data = Vertex2::get_attributes_descriptions();
		pipelineBuilder.vertex_input_info.pVertexAttributeDescriptions = data.data();
		pipelineBuilder.vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex2::get_attributes_descriptions().size());

		pip.pipeline.pipeline = pipelineBuilder.build_pipeline(GfxDevice::device);

		//clean structures
		vkDestroyShaderModule(GfxDevice::device, vs, nullptr);
		vkDestroyShaderModule(GfxDevice::device, ps, nullptr);

		return pip;
	}
}

