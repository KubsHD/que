#include "pch.h"

#include "bloom.h"

#include <gfx/renderer2.h>
#include <gfx/pipeline/builder.h>
#include <common/vk_initializers.h>
#include <gfx/rhi/vk_image.h>

struct BloomPushConstants {
	Vec2 srcResolution;
};

BloomPushConstants pc;

void BloomEffect::init(Renderer2* r2)
{
	DescriptorLayoutBuilder layout_builder;
	layout_builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	layout_builder.add_binding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

	bloom_set_layout = layout_builder.build(GfxDevice::device, VK_SHADER_STAGE_COMPUTE_BIT);
	bloom_set = r2->global_descriptor_allocator.allocate(GfxDevice::device, bloom_set_layout);

	VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

	VkPushConstantRange range{};
	range.offset = 0;
	range.size = sizeof(BloomPushConstants);
	range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	pipeline_layout_info.pPushConstantRanges = &range;
	pipeline_layout_info.pushConstantRangeCount = 1;

	auto sets = { bloom_set_layout };

	pipeline_layout_info.pSetLayouts = sets.begin();
	pipeline_layout_info.setLayoutCount = sets.size();

	VULKAN_CHECK_NOMSG(vkCreatePipelineLayout(GfxDevice::device, &pipeline_layout_info, nullptr, &bloom_downscale_pipeline.layout));

	ComputePipelineBuilder builder;

	auto cs = PipelineBuilder::load_shader_module("shader/effect/bloom_downscale.cs_c", GfxDevice::device);

	builder.set_shader(cs);
	builder.pipeline_layout = bloom_downscale_pipeline.layout;

	bloom_downscale_pipeline.pipeline = builder.build_pipeline(GfxDevice::device);

	vkDestroyShaderModule(GfxDevice::device, cs, nullptr);
	
	// create temp image with rendering resolution and format

	temp = GfxDevice::create_image(VkExtent2D{ r2->depth_image.size.width, r2->depth_image.size.height }, r2->color_format, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	
	pc.srcResolution = { (float)temp.size.width, (float)temp.size.height };

	r2->main_deletion_queue.push_function([=]() {
		vkDestroyDescriptorSetLayout(GfxDevice::device, bloom_set_layout, nullptr);
		vkDestroyPipeline(GfxDevice::device, bloom_downscale_pipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(GfxDevice::device, bloom_downscale_pipeline.layout, nullptr);

		GfxDevice::destroy_image(temp);
	});
}


void BloomEffect::render(VkCommandBuffer cmd, VkImageView inout_texture)
{
	// copy 
	DescriptorWriter writer;
	writer.write_storage_image(0, inout_texture, VK_IMAGE_LAYOUT_GENERAL);
	writer.write_storage_image(1, temp.view , VK_IMAGE_LAYOUT_GENERAL);

	writer.update_set(GfxDevice::device, bloom_set);

	vkCmdPushConstants(cmd, bloom_downscale_pipeline.layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(BloomPushConstants), &pc);
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, bloom_downscale_pipeline.pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, bloom_downscale_pipeline.layout, 0, 1, &bloom_set, 0, nullptr);
	vkCmdDispatch(cmd, temp.size.width / 16, temp.size.height / 16, 1);

	// wait for the compute shader to finish
	VkMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

void BloomEffect::update()
{
	if (ImGui::Begin("Bloom"))
	{
		ImGui::End();
	}
}
