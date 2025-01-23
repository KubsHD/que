#include "pch.h"

#include "bloom.h"

#include <gfx/renderer2.h>
#include <gfx/pipeline/builder.h>
#include <common/vk_initializers.h>
#include <gfx/rhi/vk_image.h>
#include <tracy/TracyVulkan.hpp>

extern tracy::VkCtx* ctx;


struct BloomPushConstants {
	Vec2 srcResolution;
};


BloomPushConstants pc;

Vec2 original_size;

void BloomEffect::init(Renderer2* r2)
{
	
	sampler = r2->default_sampler_linear_clamp;

	temp = GfxDevice::create_image(VkExtent2D{ r2->depth_image.size.width, r2->depth_image.size.height }, r2->color_format, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	GfxDevice::set_debug_name(temp.image, "Intermiedate bloom image");
	GfxDevice::set_debug_name(temp.view, "Intermiedate bloom image view");

	{
		// downscale pipeline
		DescriptorLayoutBuilder layout_builder;
		layout_builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		layout_builder.add_binding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

		bloom_set_layout = layout_builder.build(GfxDevice::device, VK_SHADER_STAGE_COMPUTE_BIT);

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

	}

	{
		// upscale pipeline

		DescriptorLayoutBuilder layout_builder;

		// prev mip
		layout_builder.add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		// current mip for sampling
		layout_builder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		// next mip read write
		layout_builder.add_binding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

		bloom_upscale_set_layout = layout_builder.build(GfxDevice::device, VK_SHADER_STAGE_COMPUTE_BIT);

		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();






		VkPushConstantRange range{};
		range.offset = 0;
		range.size = sizeof(BloomPushConstants);
		range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		pipeline_layout_info.pPushConstantRanges = &range;
		pipeline_layout_info.pushConstantRangeCount = 1;

		auto sets = { bloom_upscale_set_layout };

		pipeline_layout_info.pSetLayouts = sets.begin();
		pipeline_layout_info.setLayoutCount = sets.size();

		VULKAN_CHECK_NOMSG(vkCreatePipelineLayout(GfxDevice::device, &pipeline_layout_info, nullptr, &bloom_upscale_pipeline.layout));

		ComputePipelineBuilder builder;

		auto cs = PipelineBuilder::load_shader_module("shader/effect/bloom_upscale.cs_c", GfxDevice::device);

		builder.set_shader(cs);
		builder.pipeline_layout = bloom_upscale_pipeline.layout;

		bloom_upscale_pipeline.pipeline = builder.build_pipeline(GfxDevice::device);


		vkDestroyShaderModule(GfxDevice::device, cs, nullptr);
	}


	{
		// render pipeline

		DescriptorLayoutBuilder layout_builder;
		layout_builder.add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		layout_builder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		layout_builder.add_binding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);


		bloom_render_set_layout = layout_builder.build(GfxDevice::device, VK_SHADER_STAGE_COMPUTE_BIT);
		bloom_render_set = r2->global_descriptor_allocator.allocate(GfxDevice::device, bloom_render_set_layout);

		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

		auto sets = { bloom_render_set_layout };

		pipeline_layout_info.pSetLayouts = sets.begin();
		pipeline_layout_info.setLayoutCount = sets.size();



		VULKAN_CHECK_NOMSG(vkCreatePipelineLayout(GfxDevice::device, &pipeline_layout_info, nullptr, &bloom_render_pipeline.layout));

		ComputePipelineBuilder builder;


		auto cs = PipelineBuilder::load_shader_module("shader/effect/bloom_render.cs_c", GfxDevice::device);

		builder.set_shader(cs);
		builder.pipeline_layout = bloom_render_pipeline.layout;

		bloom_render_pipeline.pipeline = builder.build_pipeline(GfxDevice::device);

		vkDestroyShaderModule(GfxDevice::device, cs, nullptr);
	}

	original_size = { r2->depth_image.size.width, r2->depth_image.size.height };

	// generate mips
	mips.reserve(m_mip_count);


	Vec2 mip_size = { r2->depth_image.size.width, r2->depth_image.size.height };
	glm::ivec2 mip_int_size = { r2->depth_image.size.width, r2->depth_image.size.height };

	for (int i = 0; i < m_mip_count; i++)
	{
		BloomMip mip;


		mip_size *= 0.5f;
		mip_int_size /= 2;

		mip.size = mip_size;
		mip.int_size = mip_int_size;
		mip.texture = GfxDevice::create_image(VkExtent2D{ (uint32_t)mip.size.x, (uint32_t)mip.size.y }, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mip.set = r2->global_descriptor_allocator.allocate(GfxDevice::device, bloom_set_layout);
		mip.upscale_set = r2->global_descriptor_allocator.allocate(GfxDevice::device, bloom_upscale_set_layout);

		mips.push_back(mip);
	}

	r2->main_deletion_queue.push_function([=]() {
		vkDestroyDescriptorSetLayout(GfxDevice::device, bloom_set_layout, nullptr);
		vkDestroyPipeline(GfxDevice::device, bloom_downscale_pipeline.pipeline, nullptr);
		vkDestroyPipeline(GfxDevice::device, bloom_upscale_pipeline.pipeline, nullptr);
		vkDestroyPipeline(GfxDevice::device, bloom_render_pipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(GfxDevice::device, bloom_downscale_pipeline.layout, nullptr);
		vkDestroyPipelineLayout(GfxDevice::device, bloom_render_pipeline.layout, nullptr);
		vkDestroyPipelineLayout(GfxDevice::device, bloom_upscale_pipeline.layout, nullptr);

		for (auto& mip : mips)
		{
			GfxDevice::destroy_image(mip.texture);
		}

		GfxDevice::destroy_image(temp);
	});
}

void BloomEffect::render(VkCommandBuffer cmd, GPUImage input, GPUImage output)
{
	TracyVkZone(ctx, cmd, "Bloom");

	VkImageView src_image = input.view;
	pc.srcResolution = original_size;

	for (int i = 0; i < mips.size(); i++)
	{
		const auto& mip = mips[i];

		DescriptorWriter writer;
		writer.write_storage_image(0, src_image, VK_IMAGE_LAYOUT_GENERAL);
		writer.write_storage_image(1, mip.texture.view, VK_IMAGE_LAYOUT_GENERAL);

		writer.update_set(GfxDevice::device, mip.set);

		pc.srcResolution = mip.size;

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, bloom_downscale_pipeline.pipeline);

		vkCmdPushConstants(cmd, bloom_downscale_pipeline.layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(BloomPushConstants), &pc);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, bloom_downscale_pipeline.layout, 0, 1, &mip.set, 0, nullptr);
		vkCmdDispatch(cmd,
			(mip.size.x + 7) / 8, 
			(mip.size.y + 7) / 8,  
			1
		);

		// wait for the compute shader to finish
		VkMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
		 
		pc.srcResolution = mip.size;
		src_image = mip.texture.view;
	}

	for (int i = mips.size() - 1; i > 0; i--)

	{
		const auto& mip = mips[i];
		const auto& next_mip = mips[i - 1];





		vkutil::transition_image(cmd, mip.texture.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		DescriptorWriter writer;

		writer.write_image(0, mip.texture.view, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		writer.write_image(1, next_mip.texture.view, sampler, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		writer.write_storage_image(2, next_mip.texture.view, VK_IMAGE_LAYOUT_GENERAL);

		writer.update_set(GfxDevice::device, mip.upscale_set);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, bloom_upscale_pipeline.pipeline);
		vkCmdPushConstants(cmd, bloom_upscale_pipeline.layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(BloomPushConstants), &pc);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, bloom_upscale_pipeline.layout, 0, 1, &mip.upscale_set, 0, nullptr);
		vkCmdDispatch(cmd,
			(next_mip.size.x + 7) / 8,
			(next_mip.size.y + 7) / 8,
			1
		);





		VkMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
	}

	DescriptorWriter writer;
	writer.write_image(0, input.view, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.write_image(1, mips[0].texture.view, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.write_storage_image(2, output.view, VK_IMAGE_LAYOUT_GENERAL);


	writer.update_set(GfxDevice::device, bloom_render_set);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, bloom_render_pipeline.pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, bloom_render_pipeline.layout, 0, 1, &bloom_render_set, 0, nullptr);
	vkCmdDispatch(cmd, original_size.x / 16, original_size.y / 16, 1);

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

void BloomEffect::destroy()
{

}
