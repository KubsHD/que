#include "pch.h"


#include "sky.h"
#include <core/asset.h>
#include <common/vk_initializers.h>
#include <common/vk_image.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <gfx/buffers.h>
#include "pipeline/sky_irradiance_generate.h"


namespace gfx {

    void load_sky_texture_and_create_cubemap(GraphicsAPI_Vulkan& gapi, String hdriPath, Sky& s, Model cube, GraphicsAPI::Pipeline sky_pipeline)
    {
		VkImageView cubemapViews[6];
		VkFramebuffer framebuffers[6];

		s.skyImage = Asset::load_image(gapi, hdriPath, TT_HDRI);

		// 1. create skybox cubemap image
		VkImageCreateInfo imageCreateInfo = vkinit::image_create_info(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, { 512,512,1 });
		imageCreateInfo.mipLevels = 1;
		// Cube faces count as array layers in Vulkan
		imageCreateInfo.arrayLayers = 6;
		// This flag is required for cube map images
		imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VmaAllocationCreateInfo vai{};
		vai.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;

		VULKAN_CHECK_NOMSG(vmaCreateImage(gapi.GetAllocator(), &imageCreateInfo, &vai, &s.skyCubemap.image, &s.skyCubemap.allocation, nullptr));
		

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
		VULKAN_CHECK_NOMSG(vkCreateImageView(gapi.GetDevice(), &sky_cubemap_view_info, nullptr, &s.skyCubemap.view));

		gapi.SetDebugName("sky cubemap", s.skyCubemap.image);
		gapi.SetDebugName("sky cubemap view", s.skyCubemap.view);

		// change image layout for rendering

		gapi.immediate_submit([&](VkCommandBuffer cmd) {
			vkinit::transition_image(cmd, s.skyCubemap.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
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
			VULKAN_CHECK_NOMSG(vkCreateImageView(gapi.GetDevice(), &ivinfo, nullptr, &cubemapViews[i]));
		}

		for (int i = 0; i < 6; i++)
		{
			VkImageView attachments[1];
			attachments[0] = cubemapViews[i];

			VkFramebufferCreateInfo fbufCreateInfo{};
			fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbufCreateInfo.renderPass = sky_pipeline.renderPass;
			fbufCreateInfo.attachmentCount = 1;
			fbufCreateInfo.pAttachments = attachments;
			fbufCreateInfo.width = 512;
			fbufCreateInfo.height = 512;
			fbufCreateInfo.layers = 1;

			VULKAN_CHECK_NOMSG(vkCreateFramebuffer(gapi.GetDevice(), &fbufCreateInfo, nullptr, &framebuffers[i]));
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
		VULKAN_CHECK_NOMSG(vkCreateSampler(gapi.GetDevice(), &sinfo, nullptr, &sampler));

		// prepare buffer 
		gfx::SceneData sd;
		VkBuffer sd_buf = gapi.CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(gfx::SceneData), nullptr });

		gapi.immediate_submit([&](VkCommandBuffer cmd) {
			gapi.SetDebugName("equi2cube cmd", cmd);

			VkClearValue clearValues[2];
			clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			for (int i = 0; i < 6; i++)
			{
				VkRenderPassBeginInfo renderPassBeginInfo{};
				renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassBeginInfo.renderPass = sky_pipeline.renderPass;
				renderPassBeginInfo.framebuffer = framebuffers[i];
				renderPassBeginInfo.renderArea.extent.width = 512;
				renderPassBeginInfo.renderArea.extent.height = 512;
				renderPassBeginInfo.clearValueCount = 1;
				renderPassBeginInfo.pClearValues = clearValues;

				vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, sky_pipeline.pipeline);

				GPUEqui2CubeConstant c;
				c.viewProj = captureProjection * captureViews[i];

				XrVector3f pos = { 0,0,0 };
				XrVector3f scale = { 5,5,5 };
				XrQuaternionf rot{};

				vkCmdPushConstants(cmd, sky_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUEqui2CubeConstant), &c);

				VkDescriptorSet set{};

				VkDescriptorSetAllocateInfo descSetAI;
				descSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descSetAI.pNext = nullptr;
				descSetAI.descriptorPool = gapi.GetDescriptorPool();
				descSetAI.descriptorSetCount = 1;
				descSetAI.pSetLayouts = sky_pipeline.descriptorSetLayouts.data();
				VULKAN_CHECK(vkAllocateDescriptorSets(gapi.GetDevice(), &descSetAI, &set), "Failed to allocate DescriptorSet.");


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
				descImageInfo.imageView = s.skyImage.view;
				descImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descImageInfo.sampler = sampler;

				writeDescSet2.pImageInfo = &descImageInfo;

				VkPipelineLayout pipelineLayout = sky_pipeline.layout;

				std::vector<VkWriteDescriptorSet> vkWriteDescSets = { writeDescSet, writeDescSet2 };

				vkUpdateDescriptorSets(gapi.GetDevice(), static_cast<uint32_t>(vkWriteDescSets.size()), vkWriteDescSets.data(), 0, nullptr);
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
				vkCmdBindVertexBuffers(cmd, 0, 1, &cube.meshes[0].vertex_buffer, offsets);
				vkCmdBindIndexBuffer(cmd, cube.meshes[0].index_buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(cmd, cube.meshes[0].index_count, 1, 0, 0, 0);

				vkCmdEndRenderPass(cmd);
			}
			});

		// change to image layout ready to be used in a shader
		gapi.immediate_submit([&](VkCommandBuffer cmd) {
			vkinit::transition_image(cmd, s.skyCubemap.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});


    }

	void generate_irradiance_map(GraphicsAPI_Vulkan& gapi, Sky& s, Model cube, GraphicsAPI::Pipeline sky_render_pipeline)
	{
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
		VULKAN_CHECK_NOMSG(vkCreateImageView(gapi.GetDevice(), &sky_cubemap_view_info, nullptr, &s.skyIrradiance.view));

		gapi.SetDebugName("sky irradiance", s.skyIrradiance.image);
		gapi.SetDebugName("sky irradiance view", s.skyIrradiance.view);

		// change image layout for rendering

		gapi.immediate_submit([&](VkCommandBuffer cmd) {
			vkinit::transition_image(cmd, s.skyIrradiance.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
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
			VULKAN_CHECK_NOMSG(vkCreateImageView(gapi.GetDevice(), &ivinfo, nullptr, &cubemapViews[i]));
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

			VULKAN_CHECK_NOMSG(vkCreateFramebuffer(gapi.GetDevice(), &fbufCreateInfo, nullptr, &framebuffers[i]));
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
		VULKAN_CHECK_NOMSG(vkCreateSampler(gapi.GetDevice(), &sinfo, nullptr, &sampler));

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
				VULKAN_CHECK(vkAllocateDescriptorSets(gapi.GetDevice(), &descSetAI, &set), "Failed to allocate DescriptorSet.");


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

				vkUpdateDescriptorSets(gapi.GetDevice(), static_cast<uint32_t>(vkWriteDescSets.size()), vkWriteDescSets.data(), 0, nullptr);
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
				vkCmdBindVertexBuffers(cmd, 0, 1, &cube.meshes[0].vertex_buffer, offsets);
				vkCmdBindIndexBuffer(cmd, cube.meshes[0].index_buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(cmd, cube.meshes[0].index_count, 1, 0, 0, 0);

				vkCmdEndRenderPass(cmd);
			}
			});

		// change to image layout ready to be used in a shader
		gapi.immediate_submit([&](VkCommandBuffer cmd) {
			vkinit::transition_image(cmd, s.skyIrradiance.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});

	}

	Sky gfx::sky::create_sky(GraphicsAPI_Vulkan& gapi, String hdriPath, Model cube, GraphicsAPI::Pipeline sky_render_pipeline)
	{
        Sky s{};

		GraphicsAPI::Pipeline sky_irradiance_pipeline = pipeline::create_sky_irradiance_pipeline(gapi);

		load_sky_texture_and_create_cubemap(gapi, hdriPath, s, cube, sky_render_pipeline);
		generate_irradiance_map(gapi, s, cube, sky_irradiance_pipeline);
       
        return s;
	}

}
