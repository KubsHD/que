#include "pch.h"

#include "mat_sky.h"

#include <common/vk_initializers.h>

#include <gfx/renderer2.h>
#include <gfx/pipeline/builder.h>
#include <gfx/rhi/gfx_device.h>
#include <gfx/rhi/vk_helper.h>

void MAT_Sky::create(Renderer2* ren)
{
	PipelineBuilder pipelineBuilder;

	auto vs = PipelineBuilder::load_shader_module("shader/sky.vs_c", GfxDevice::device);
	auto ps = PipelineBuilder::load_shader_module("shader/sky.ps_c", GfxDevice::device);


	VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

	{
		DescriptorLayoutBuilder	builder;
		builder.add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		material_layout = builder.build(GfxDevice::device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	auto sets = { ren->scene_data_set_layout, material_layout };

	pipeline_layout_info.pSetLayouts = sets.begin();
	pipeline_layout_info.setLayoutCount = sets.size();

	VkPushConstantRange range{};
	range.offset = 0;
	range.size = sizeof(GPUDrawPushConstants);
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	pipeline_layout_info.pPushConstantRanges = &range;
	pipeline_layout_info.pushConstantRangeCount = 1;

	VULKAN_CHECK_NOMSG(vkCreatePipelineLayout(GfxDevice::device, &pipeline_layout_info, nullptr, &this->pipeline.layout));

	//use the triangle layout we created
	pipelineBuilder.pipeline_layout = this->pipeline.layout;
	pipelineBuilder.set_shaders(vs, ps);
	pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.set_multisampling_none();
	pipelineBuilder.disable_blending();
	pipelineBuilder.enable_depthtest(true, VK_COMPARE_OP_LESS);

	//connect the image format we will draw into, from draw image
	pipelineBuilder.set_color_attachment_format(VK_FORMAT_R8G8B8A8_SRGB);
	pipelineBuilder.set_depth_format(ren->depth_format);

	pipelineBuilder.vertex_input_info.vertexBindingDescriptionCount = 1;

	// referencing this directly crashes vkCreateGraphicsPipelines in release mode
	auto bdata = Vertex2::get_binding_description();
	pipelineBuilder.vertex_input_info.pVertexBindingDescriptions = &bdata;

	pipelineBuilder.vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex2::get_attributes_descriptions().size());

	// referencing this directly crashes vkCreateGraphicsPipelines in release mode
	auto data = Vertex2::get_attributes_descriptions();
	pipelineBuilder.vertex_input_info.pVertexAttributeDescriptions = data.data();

	//finally build the pipeline
	this->pipeline.pipeline = pipelineBuilder.build_pipeline(GfxDevice::device);

	//clean structures
	vkDestroyShaderModule(GfxDevice::device, vs, nullptr);
	vkDestroyShaderModule(GfxDevice::device, ps, nullptr);

	ren->main_deletion_queue.push_function([&]() {
		vkDestroyPipelineLayout(GfxDevice::device, this->pipeline.layout, nullptr);
		vkDestroyPipeline(GfxDevice::device, this->pipeline.pipeline, nullptr);
	});
}

void MAT_Sky::clear(VkDevice device)
{
	vkDestroyDescriptorSetLayout(device, this->material_layout, nullptr);
}

MaterialInstance MAT_Sky::write(VkDevice device, const Resoruces& res, DescriptorAllocator* allocator)
{
	MaterialInstance instance;

	instance.pipeline = &this->pipeline;
	instance.material_set = allocator->allocate(device, this->material_layout);

	DescriptorWriter writer;
	writer.write_image(0, res.sky_cubemap.view, res.sky_cubemap_sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.update_set(device, instance.material_set);

	return instance;
}
