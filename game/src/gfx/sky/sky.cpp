#include "pch.h"


#include "sky.h"
#include <common/vk_initializers.h>
#include <gfx/rhi/vk_image.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <gfx/buffers.h>

#include "sky_irradiance_generate.h"
#include "sky_cube_render_pipeline.h"
#include "sky_pipeline.h"

#include <core/profiler.h>
#include <asset/asset_manager.h>
#include <gfx/rhi/vk_helper.h>
#include <gfx/rhi/vk_descriptor.h>
#include <gfx/renderer2.h>
#include <asset/file/texture.h>
#include <lib/dds-ktx.h>

namespace gfx {

    void load_sky_texture_and_create_cubemap(Renderer2& ren, String hdriPath, Sky& s, Model cube, SkyIntermiedatePipeline cube_render_pipeline)
    {
		QUE_PROFILE;

		VkImageView cubemapViews[6];
		VkFramebuffer framebuffers[6];

		s.skyImage = AssetManager::load_texture(hdriPath, TT_HDRI);

		// 1. create skybox cubemap image
		VkImageCreateInfo imageCreateInfo = vkinit::image_create_info(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, { 512,512,1 });
		imageCreateInfo.mipLevels = 1;
		// Cube faces count as array layers in Vulkan
		imageCreateInfo.arrayLayers = 6;
		// This flag is required for cube map images
		imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VmaAllocationCreateInfo vai{};
		vai.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;

		VULKAN_CHECK_NOMSG(vmaCreateImage(GfxDevice::allocator, &imageCreateInfo, &vai, &s.skyCubemap.image, &s.skyCubemap.allocation, nullptr));
		
		// create cubemap view
		VkImageViewCreateInfo sky_cubemap_view_info = {};
		sky_cubemap_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		sky_cubemap_view_info.pNext = nullptr;
		sky_cubemap_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		sky_cubemap_view_info.image = s.skyCubemap.image;
		sky_cubemap_view_info.format = VK_FORMAT_R16G16B16A16_SFLOAT;
		sky_cubemap_view_info.subresourceRange.baseMipLevel = 0;
		sky_cubemap_view_info.subresourceRange.levelCount = 1;
		sky_cubemap_view_info.subresourceRange.baseArrayLayer = 0;
		sky_cubemap_view_info.subresourceRange.layerCount = 6;
		sky_cubemap_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VULKAN_CHECK_NOMSG(vkCreateImageView(GfxDevice::device, &sky_cubemap_view_info, nullptr, &s.skyCubemap.view));

		GfxDevice::set_debug_name(s.skyCubemap.image, "sky cubemap");
		GfxDevice::set_debug_name(s.skyCubemap.view, "sky cubemap view");


		
		// change image layout for rendering

		GfxDevice::immediate_submit([&](VkCommandBuffer cmd) {
			vkutil::transition_image(cmd, s.skyCubemap.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		});

		// create view for the image

		for (int i = 0; i < 6; i++)
		{
			VkImageViewCreateInfo ivinfo = {};
			ivinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ivinfo.pNext = nullptr;
			ivinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			ivinfo.image = s.skyCubemap.image;
			ivinfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
			ivinfo.subresourceRange.baseMipLevel = 0;
			ivinfo.subresourceRange.levelCount = 1;
			ivinfo.subresourceRange.baseArrayLayer = i;
			ivinfo.subresourceRange.layerCount = 1;
			ivinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			VULKAN_CHECK_NOMSG(vkCreateImageView(GfxDevice::device, &ivinfo, nullptr, &cubemapViews[i]));
		}

		// rendering to cube sides
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		GfxDevice::immediate_submit([&](VkCommandBuffer cmd) {
			GfxDevice::set_debug_name(cmd, "equi2cube cmd");

			VkClearValue clearValues;
			clearValues.color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues.depthStencil = { 1.0f, 0 };

			std::vector<VkDescriptorSet> sets;

			for (int i = 0; i < 6; i++)
			{
				VkRenderingAttachmentInfo color_attachment = vkinit::attachment_info(cubemapViews[i], &clearValues, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

				VkRenderingInfo renderInfo = vkinit::rendering_info({ 512, 512 }, &color_attachment, nullptr);

				vkCmdBeginRendering(cmd, &renderInfo);

				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, cube_render_pipeline.pipeline.pipeline);

				GPUEqui2CubeConstant c;
				c.viewProj = captureProjection * captureViews[i];

				XrVector3f pos = { 0,0,0 };
				XrVector3f scale = { 5,5,5 };
				XrQuaternionf rot{};

				vkCmdPushConstants(cmd, cube_render_pipeline.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GPUEqui2CubeConstant), &c);

				sets.push_back(ren.global_descriptor_allocator.allocate(GfxDevice::device, cube_render_pipeline.descriptor_set_layout));

				{
					DescriptorWriter writer;
					writer.write_image(0, s.skyImage.view, ren.default_sampler_linear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					writer.update_set(GfxDevice::device, sets.back());
				}

				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, cube_render_pipeline.pipeline.layout, 0, 1, &sets.back(), 0, nullptr);

				std::vector<VkViewport> vkViewports;
				vkViewports.reserve(1);
				vkViewports.push_back({ 0,0,512,512, 0, 1 });
				vkCmdSetViewport(cmd, 0, static_cast<uint32_t>(vkViewports.size()), vkViewports.data());

				std::vector<VkRect2D> vkRect2D;
				vkRect2D.reserve(1);
				vkRect2D.push_back({ {0,0}, {512,512} });
				vkCmdSetScissor(cmd, 0, static_cast<uint32_t>(vkRect2D.size()), vkRect2D.data());

				VkDeviceSize offsets[] = { 0 };
			    vkCmdBindVertexBuffers(cmd, 0, 1, &cube.meshes[0].vertex_buffer.buffer, offsets);
				vkCmdBindIndexBuffer(cmd, cube.meshes[0].index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(cmd, cube.meshes[0].index_count, 1, 0, 0, 0);

				vkCmdEndRendering(cmd);
			}
		});

		// change to image layout ready to be used in a shader
		GfxDevice::immediate_submit([&](VkCommandBuffer cmd) {
			vkutil::transition_image(cmd, s.skyCubemap.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});

		for (int i = 0; i < 6; i++)
		{
			vkDestroyImageView(GfxDevice::device, cubemapViews[i], nullptr);
		}
    }

	void load_compiled_sky_texture(Renderer2& ren, String cskyPath, Sky& s)
	{

		// 1. create skybox cubemap image
		VkImageCreateInfo imageCreateInfo = vkinit::image_create_info(VK_FORMAT_BC6H_SFLOAT_BLOCK, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, { 2048,2048,1 });
		imageCreateInfo.mipLevels = 1;
		// Cube faces count as array layers in Vulkan
		imageCreateInfo.arrayLayers = 6;
		// This flag is required for cube map images
		imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VmaAllocationCreateInfo vai{};
		vai.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;

		VULKAN_CHECK_NOMSG(vmaCreateImage(GfxDevice::allocator, &imageCreateInfo, &vai, &s.skyCubemap.image, &s.skyCubemap.allocation, nullptr));

		// create cubemap view
		VkImageViewCreateInfo sky_cubemap_view_info = {};
		sky_cubemap_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		sky_cubemap_view_info.pNext = nullptr;
		sky_cubemap_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		sky_cubemap_view_info.image = s.skyCubemap.image;
		sky_cubemap_view_info.format = VK_FORMAT_BC6H_SFLOAT_BLOCK;
		sky_cubemap_view_info.subresourceRange.baseMipLevel = 0;
		sky_cubemap_view_info.subresourceRange.levelCount = 1;
		sky_cubemap_view_info.subresourceRange.baseArrayLayer = 0;
		sky_cubemap_view_info.subresourceRange.layerCount = 6;
		sky_cubemap_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VULKAN_CHECK_NOMSG(vkCreateImageView(GfxDevice::device, &sky_cubemap_view_info, nullptr, &s.skyCubemap.view));


		auto imageData = AssetManager::read_all_bytes(cskyPath);
		C_Texture tex;
		tex.read(imageData);

		ddsktx_texture_info info;
		ddsktx_parse(&info, tex.dds_blob, tex.blob_size);

		s.skyCubemap.size.width = info.width;
		s.skyCubemap.size.height = info.height;

		ddsktx_sub_data sub_data;
		ddsktx_get_sub(&info, &sub_data, tex.dds_blob, tex.blob_size, 0, 0, 0);


		assert((info.flags & DDSKTX_TEXTURE_FLAG_CUBEMAP) != 0);

		for (int face = 0; face < DDSKTX_CUBE_FACE_COUNT; face++) {
            ddsktx_sub_data sub_data;
            ddsktx_get_sub(&info, &sub_data, tex.dds_blob, tex.blob_size, 0, face, 0);

			GfxDevice::upload_image(s.skyCubemap, (void*)sub_data.buff, sub_data.size_bytes, false, face);
        }
	}

	void generate_irradiance_map(GraphicsAPI_Vulkan& gapi, Sky& s, Model cube, GraphicsAPI::Pipeline sky_render_pipeline)
	{
		QUE_PROFILE;

		VkImageView cubemapViews[6];
		VkFramebuffer framebuffers[6];

		// 1. create skybox cubemap image
		VkImageCreateInfo imageCreateInfo = vkinit::image_create_info(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, { 512,512,1 });
		imageCreateInfo.mipLevels = 1;
		// Cube faces count as array layers in Vulkan
		imageCreateInfo.arrayLayers = 6;
		// This flag is required for cube map images
		imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VmaAllocationCreateInfo vai{};
		vai.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;

		VULKAN_CHECK_NOMSG(vmaCreateImage(gapi.GetAllocator(), &imageCreateInfo, &vai, &s.skyIrradiance.image, &s.skyIrradiance.allocation, nullptr));


		// create cubemap view
		VkImageViewCreateInfo sky_cubemap_view_info = {};
		sky_cubemap_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		sky_cubemap_view_info.pNext = nullptr;
		sky_cubemap_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		sky_cubemap_view_info.image = s.skyIrradiance.image;
		sky_cubemap_view_info.format = VK_FORMAT_R16G16B16A16_SFLOAT;
		sky_cubemap_view_info.subresourceRange.baseMipLevel = 0;
		sky_cubemap_view_info.subresourceRange.levelCount = 1;
		sky_cubemap_view_info.subresourceRange.baseArrayLayer = 0;
		sky_cubemap_view_info.subresourceRange.layerCount = 6;
		sky_cubemap_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VULKAN_CHECK_NOMSG(vkCreateImageView(GfxDevice::device, &sky_cubemap_view_info, nullptr, &s.skyIrradiance.view));

		gapi.SetDebugName("sky irradiance", s.skyIrradiance.image);
		gapi.SetDebugName("sky irradiance view", s.skyIrradiance.view);

		// change image layout for rendering

		gapi.immediate_submit([&](VkCommandBuffer cmd) {
			vkutil::transition_image(cmd, s.skyIrradiance.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		});

		// create view for the image

		for (int i = 0; i < 6; i++)
		{
			VkImageViewCreateInfo ivinfo = {};
			ivinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ivinfo.pNext = nullptr;
			ivinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			ivinfo.image = s.skyIrradiance.image;
			ivinfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
			ivinfo.subresourceRange.baseMipLevel = 0;
			ivinfo.subresourceRange.levelCount = 1;
			ivinfo.subresourceRange.baseArrayLayer = i;
			ivinfo.subresourceRange.layerCount = 1;
			ivinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			VULKAN_CHECK_NOMSG(vkCreateImageView(GfxDevice::device, &ivinfo, nullptr, &cubemapViews[i]));
		}

		for (int i = 0; i < 6; i++)
		{
			VkImageView attachments[1];
			attachments[0] = cubemapViews[i];

			VkFramebufferCreateInfo fbufCreateInfo{};
			fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbufCreateInfo.renderPass = sky_render_pipeline.renderPass;
			fbufCreateInfo.attachmentCount = 1;
			fbufCreateInfo.pAttachments = attachments;
			fbufCreateInfo.width = 512;
			fbufCreateInfo.height = 512;
			fbufCreateInfo.layers = 1;

			VULKAN_CHECK_NOMSG(vkCreateFramebuffer(GfxDevice::device, &fbufCreateInfo, nullptr, &framebuffers[i]));
		}

		// rendering to cube sides

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		VkSampler sampler;
		VkSamplerCreateInfo sinfo = vkinit::sampler_create_info(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		VULKAN_CHECK_NOMSG(vkCreateSampler(GfxDevice::device, &sinfo, nullptr, &sampler));

		// prepare buffer 
		gfx::SceneData sd;
		VkBuffer sd_buf = gapi.CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(gfx::SceneData), nullptr });

		gapi.immediate_submit([&](VkCommandBuffer cmd) {
			gapi.SetDebugName("irradiance cmd", cmd);

			VkClearValue clearValues[2];
			clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			for (int i = 0; i < 6; i++)
			{
				VkRenderPassBeginInfo renderPassBeginInfo{};
				renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassBeginInfo.renderPass = sky_render_pipeline.renderPass;
				renderPassBeginInfo.framebuffer = framebuffers[i];
				renderPassBeginInfo.renderArea.extent.width = 512;
				renderPassBeginInfo.renderArea.extent.height = 512;
				renderPassBeginInfo.clearValueCount = 1;
				renderPassBeginInfo.pClearValues = clearValues;

				vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, sky_render_pipeline.pipeline);

				GPUEqui2CubeConstant c;
				c.viewProj = captureProjection * captureViews[i];

				XrVector3f pos = { 0,0,0 };
				XrVector3f scale = { 5,5,5 };
				XrQuaternionf rot{};

				vkCmdPushConstants(cmd, sky_render_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUEqui2CubeConstant), &c);

				VkDescriptorSet set{};

				VkDescriptorSetAllocateInfo descSetAI;
				descSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descSetAI.pNext = nullptr;
				descSetAI.descriptorPool = gapi.GetDescriptorPool();
				descSetAI.descriptorSetCount = 1;
				descSetAI.pSetLayouts = sky_render_pipeline.descriptorSetLayouts.data();
				VULKAN_CHECK(vkAllocateDescriptorSets(GfxDevice::device, &descSetAI, &set), "Failed to allocate DescriptorSet.");


				// desc 0
				VkWriteDescriptorSet writeDescSet{};
				writeDescSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescSet.pNext = nullptr;
				writeDescSet.dstSet = set;
				writeDescSet.dstBinding = 0;
				writeDescSet.dstArrayElement = 0;
				writeDescSet.descriptorCount = 1;
				writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescSet.pImageInfo = nullptr;
				writeDescSet.pTexelBufferView = nullptr;


				VkDescriptorBufferInfo descBufferInfo{};
				descBufferInfo.buffer = sd_buf;
				descBufferInfo.offset = 0;
				descBufferInfo.range = sizeof(gfx::SceneData);
				writeDescSet.pBufferInfo = &descBufferInfo;

				//desc 1
				VkWriteDescriptorSet writeDescSet2{};
				writeDescSet2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescSet2.pNext = nullptr;
				writeDescSet2.dstSet = set;
				writeDescSet2.dstBinding = 1;
				writeDescSet2.dstArrayElement = 0;
				writeDescSet2.descriptorCount = 1;
				writeDescSet2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescSet2.pTexelBufferView = nullptr;


				VkDescriptorImageInfo descImageInfo{};
				descImageInfo.imageView = s.skyCubemap.view;
				descImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descImageInfo.sampler = sampler;

				writeDescSet2.pImageInfo = &descImageInfo;


				VkPipelineLayout pipelineLayout = sky_render_pipeline.layout;

				std::vector<VkWriteDescriptorSet> vkWriteDescSets = { writeDescSet, writeDescSet2 };

				vkUpdateDescriptorSets(GfxDevice::device, static_cast<uint32_t>(vkWriteDescSets.size()), vkWriteDescSets.data(), 0, nullptr);
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set, 0, nullptr);

				std::vector<VkViewport> vkViewports;
				vkViewports.reserve(1);
				vkViewports.push_back({ 0,0,512,512, 0, 1 });
				vkCmdSetViewport(cmd, 0, static_cast<uint32_t>(vkViewports.size()), vkViewports.data());

				std::vector<VkRect2D> vkRect2D;
				vkRect2D.reserve(1);
				vkRect2D.push_back({ {0,0}, {512,512} });
				vkCmdSetScissor(cmd, 0, static_cast<uint32_t>(vkRect2D.size()), vkRect2D.data());

				VkDeviceSize offsets[] = { 0 };
		/*		vkCmdBindVertexBuffers(cmd, 0, 1, &cube.meshes[0].vertex_buffer, offsets);
				vkCmdBindIndexBuffer(cmd, cube.meshes[0].index_buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(cmd, cube.meshes[0].index_count, 1, 0, 0, 0);*/

				vkCmdEndRenderPass(cmd);
			}
			});

		// change to image layout ready to be used in a shader
		gapi.immediate_submit([&](VkCommandBuffer cmd) {
			vkutil::transition_image(cmd, s.skyIrradiance.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});

	}

	void Sky::create(Renderer2& ren, String hdriPath, String cube)
	{
		QUE_PROFILE;
		
		this->mat_sky.create(&ren);

		this->skybox_cube = AssetManager::load_model(cube);
		auto sky_cube_render_pipeline = pipeline::create_sky_cube_render_pipeline(ren);
		//load_sky_texture_and_create_cubemap(ren, hdriPath, *this, this->skybox_cube, sky_cube_render_pipeline);
		load_compiled_sky_texture(ren, hdriPath, *this);

		//auto sip = pipeline::create_sky_irradiance_pipeline(ren);
		/*generate_irradiance_map(gapi, s, cube, sky_irradiance_pipeline); */

		MAT_Sky::Resoruces res;
		res.sky_cubemap = this->skyCubemap;
		res.sky_cubemap_sampler = ren.default_sampler_linear;
		
		this->mat_sky_instance = this->mat_sky.write(GfxDevice::device, res, &ren.global_descriptor_allocator);

		sky_cube_render_pipeline.clear(GfxDevice::device);
	}

	void Sky::clear(VkDevice device)
	{
		this->mat_sky.clear(GfxDevice::device);

		vmaDestroyImage(GfxDevice::allocator, this->skyCubemap.image, this->skyCubemap.allocation);
		vkDestroyImageView(GfxDevice::device, this->skyCubemap.view, nullptr);

		//vmaDestroyImage(GfxDevice::allocator, this->skyIrradiance.image, this->skyIrradiance.allocation);
		//vkDestroyImageView(GfxDevice::device, this->skyIrradiance.view, nullptr);

	}

	void Sky::draw(Renderer2& ren, VkCommandBuffer cmd)
	{
		QUE_GPU_ZONE(cmd, "Sky draw");

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mat_sky_instance.pipeline->pipeline);

		GPUDrawPushConstants pushConst;
		pushConst.model = glm::mat4(1.0f);
		pushConst.model = glm::scale(pushConst.model, glm::vec3(20.0f, 20.0f, 20.0f));

		vkCmdPushConstants(cmd, mat_sky_instance.pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GPUDrawPushConstants), &pushConst);

		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mat_sky_instance.pipeline->layout, 0, 1, &ren.scene_data_set, 0, nullptr);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mat_sky_instance.pipeline->layout, 1, 1, &mat_sky_instance.material_set, 0, nullptr);

		VkDeviceSize offsets = { 0 };

		vkCmdBindVertexBuffers(cmd, 0, 1, &skybox_cube.meshes[0].vertex_buffer.buffer, &offsets);
		vkCmdBindIndexBuffer(cmd, skybox_cube.meshes[0].index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, skybox_cube.meshes[0].index_count, 1, 0, 0, 0);
	}

}
