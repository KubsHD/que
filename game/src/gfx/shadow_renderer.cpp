#include "pch.h"

#include "shadow_renderer.h"
#include "pipeline/dir_light_pipeline.h"

#include <gfx/renderer2.h>
#include <common/vk_initializers.h>
#include <core/components/components.h>
#include <core/systems/base_system.h>
#include "light.h"
#include "rhi/vk_image.h"
#include <tracy/TracyVulkan.hpp>
#include <core/profiler.h>

VkExtent2D shadow_map_size = { 8192, 8192 };

void ShadowRenderer::create(Renderer2& ren)
{
	directional_shadow_map = GfxDevice::create_image(shadow_map_size, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	dir_light_sm_pipeline = pipeline::create_dir_light_pipeline(ren);

	// we need a seperate sampler to not repeat the shadow map when sampling 
	VkSamplerCreateInfo sampl{};
	sampl.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampl.pNext = nullptr;

	sampl.magFilter = VK_FILTER_LINEAR;
	sampl.minFilter = VK_FILTER_LINEAR;
	sampl.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampl.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampl.addressModeV = sampl.addressModeU;
	sampl.addressModeW = sampl.addressModeU;
	sampl.mipLodBias = 0.0f;
	sampl.maxAnisotropy = 1.0f;
	sampl.minLod = 0.0f;
	sampl.maxLod = 1.0f;
	sampl.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	vkCreateSampler(GfxDevice::device, &sampl, nullptr, &shadow_map_sampler);

	GfxDevice::set_debug_name(directional_shadow_map.image, "directionaL_light_shadow_map");
}

void ShadowRenderer::update()
{
	render_imgui();


}

void ShadowRenderer::render(VkCommandBuffer cmd, entt::registry& reg, glm::vec3 cam_pos)
{
	QUE_GPU_ZONE(cmd, "Shadow Rendering");

	gfx::DirectionalLight dl;
	dl.direction = glm::vec3(2.0f, 5.0f, 1.0f);

	VkRenderingAttachmentInfo depthAttachment = vkinit::depth_attachment_info(directional_shadow_map.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	VkClearValue clearValues[2];
	depthAttachment.clearValue.depthStencil.depth = 1.f;

	VkExtent2D _windowExtent = shadow_map_size;
	VkRenderingInfo render_info = vkinit::rendering_info(_windowExtent, nullptr, &depthAttachment);

	vkCmdBeginRendering(cmd, &render_info);

	//set dynamic viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = shadow_map_size.width;
	viewport.height = shadow_map_size.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = shadow_map_size.width;
	scissor.extent.height = shadow_map_size.height;

	vkCmdSetScissor(cmd, 0, 1, &scissor);
	VkDeviceSize offset = { 0 };


	glm::mat4 lightProjection, lightView;


	
	lightProjection = glm::ortho(-frustum_size, frustum_size, -frustum_size, frustum_size, near_plane, far_plane);

	// https://www.reddit.com/r/vulkan/comments/y74ij3/why_is_it_that_i_need_to_invert_the_projection/
	lightProjection[1][1] *= -1;

	lightView = glm::lookAt(glm::vec3(0.5f, dl_height, 2), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

	GPUDrawPushConstants pc;
	light_mtx = lightProjection * lightView;

	auto modelsToRender = reg.view<core_transform_component, core_mesh_component>();
	for (const auto&& [e, tc, mc] : modelsToRender.each())
	{
		tc.calculate_matrix();
		glm::mat4 model_matrix = tc.matrix;

		pc.model = light_mtx * model_matrix;

		for (auto& mesh : mc.model->meshes)
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, dir_light_sm_pipeline.pipeline);

			vkCmdSetDepthBias(cmd, 1.25f, 0.0f, 1.75f);

			vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertex_buffer.buffer, &offset);
			vkCmdBindIndexBuffer(cmd, mesh.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdPushConstants(cmd, dir_light_sm_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GPUDrawPushConstants), &pc);

			vkCmdDrawIndexed(cmd, mesh.index_count, 1, 0, 0, 0);
		}
	};

	vkCmdEndRendering(cmd);

	//transition the shadow map image for sampling
	vkutil::transition_image(cmd, directional_shadow_map.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, -1, true);
}

void ShadowRenderer::render_imgui()
{
	if (ImGui::Begin("Shadow Rendering"))
	{
		ImGui::SliderFloat("Near Plane", &near_plane, 0.001f, 10.0f);
		ImGui::SliderFloat("Far Plane", &far_plane, 10.0f, 100.0f);
		ImGui::SliderFloat("Frustum Size", &frustum_size, 1.0f, 100.0f);
		ImGui::SliderFloat("Directional Light Height", &dl_height, 1.0f, 30.0f);
		ImGui::End();
	}

}

void ShadowRenderer::destroy()
{
	GfxDevice::destroy_image(directional_shadow_map);
	vkDestroyPipelineLayout(GfxDevice::device, dir_light_sm_pipeline.layout, nullptr);
	GfxDevice::destroy_pipeline(dir_light_sm_pipeline.pipeline);
	vkDestroySampler(GfxDevice::device, shadow_map_sampler, nullptr);
	
}
