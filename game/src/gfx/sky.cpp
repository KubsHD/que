#include "sky.h"
#include <core/asset.h>
#include <common/vk_initializers.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace gfx {

	Sky gfx::sky::create_sky(GraphicsAPI_Vulkan& gapi, String hdriPath, Model cube, VkPipeline sky_pipeline)
	{
		Sky s;

        VkImageView cubemapViews[6];
        VkFramebuffer framebuffers[6];

		s.skyImage = Asset::load_image(gapi, hdriPath, TT_HDRI);

        // 1. create skybox cubemap image
        GraphicsAPI::Image sky_cube;

        VkImageCreateInfo imageCreateInfo = vkinit::image_create_info(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, { 512,512,1 });
        
        imageCreateInfo.mipLevels = 1;
        // Cube faces count as array layers in Vulkan
        imageCreateInfo.arrayLayers = 6;
        // This flag is required for cube map images
        imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        VmaAllocationCreateInfo vai{};
        vai.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;

        VULKAN_CHECK_NOMSG(vmaCreateImage(gapi.GetAllocator(), &imageCreateInfo, &vai, &sky_cube.image, &sky_cube.allocation, nullptr));
        
        // change image layout

        gapi.immediate_submit([&](VkCommandBuffer cmd) {
            VkImageSubresourceRange range{};
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            range.baseMipLevel = 0;
            range.baseArrayLayer = 0;
            range.levelCount = 1;
            range.layerCount = 6;

            VkImageMemoryBarrier imageBarrier_toTransfer = {};
            imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

            imageBarrier_toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarrier_toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageBarrier_toTransfer.image = sky_cube.image;
            imageBarrier_toTransfer.subresourceRange = range;

            imageBarrier_toTransfer.srcAccessMask = 0;
            imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

            });



        // create view for the image

        for (int i = 0; i < 6; i++)
        {
            VkImageViewCreateInfo ivinfo = {};
            ivinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ivinfo.pNext = nullptr;
            ivinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ivinfo.image = sky_cube.image;
            ivinfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            ivinfo.subresourceRange.baseMipLevel = 0;
            ivinfo.subresourceRange.levelCount = 1;
            ivinfo.subresourceRange.baseArrayLayer = i;
            ivinfo.subresourceRange.layerCount = 1;
            ivinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vkCreateImageView(gapi.GetDevice(), &ivinfo, nullptr, &cubemapViews[i]);
        }

		// create sampler cube
        VkAttachmentDescription colorAttDesc{};
        colorAttDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        colorAttDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference colorAttRef{};
        colorAttRef.attachment = 0;
        colorAttRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription;
        subpassDescription.flags = static_cast<VkSubpassDescriptionFlags>(0);
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttRef;
        subpassDescription.pResolveAttachments = nullptr;
        subpassDescription.pDepthStencilAttachment = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;

        VkSubpassDependency subpassDependency;
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = VkAccessFlagBits(0);
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependency.dependencyFlags = VkDependencyFlagBits(0);

        VkRenderPass renderPass{};
        VkRenderPassCreateInfo renderPassCI;
        renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCI.pNext = nullptr;
        renderPassCI.flags = 0;
        renderPassCI.attachmentCount = 1;
        renderPassCI.pAttachments = &colorAttDesc;
        renderPassCI.subpassCount = 1;
        renderPassCI.pSubpasses = &subpassDescription;
        renderPassCI.dependencyCount = 1;
        renderPassCI.pDependencies = &subpassDependency;
        VULKAN_CHECK(vkCreateRenderPass(gapi.GetDevice(), &renderPassCI, nullptr, &renderPass), "Failed to create RenderPass.");

     

        for (int i = 0; i < 6; i++)
        {
            VkImageView attachments[2];
            attachments[0] = cubemapViews[i];

            VkFramebufferCreateInfo fbufCreateInfo{};
            fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbufCreateInfo.renderPass = renderPass;
            fbufCreateInfo.attachmentCount = 1;
            fbufCreateInfo.pAttachments = attachments;
            fbufCreateInfo.width = 512;
            fbufCreateInfo.height = 512;
            fbufCreateInfo.layers = 1;

            vkCreateFramebuffer(gapi.GetDevice(), &fbufCreateInfo, nullptr, &framebuffers[i]);
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


        gapi.immediate_submit([&](VkCommandBuffer cmd) {
            VkClearValue clearValues[2];
            clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
            clearValues[1].depthStencil = { 1.0f, 0 };

            for (int i = 0; i < 6; i++)
            {
                VkRenderPassBeginInfo renderPassBeginInfo{};
                renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassBeginInfo.renderPass = renderPass;
                renderPassBeginInfo.framebuffer = framebuffers[i];
                renderPassBeginInfo.renderArea.extent.width = 512;
                renderPassBeginInfo.renderArea.extent.height = 512;
                renderPassBeginInfo.clearValueCount = 1;
                renderPassBeginInfo.pClearValues = clearValues;

                vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, sky_pipeline);
                
                auto projView = captureProjection * captureViews[i];

                
       /*         
                vkCmdBindVertexBuffers(cmd, 0, 1, &cube.meshes[0].vertex_buffer, nullptr);
                vkCmdBindIndexBuffer(cmd, cube.meshes[0].index_buffer, 0, VK_INDEX_TYPE_UINT16);
                vkCmdDrawIndexed(cmd, cube.meshes[0].index_count, 1, 0, 0, 0);*/

                vkCmdEndRenderPass(cmd);
            }
        });

		return s;
	}

}
