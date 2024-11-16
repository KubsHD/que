#include "pch.h"

#include "shadow_renderer.h"
#include "pipeline/dir_light_pipeline.h"

#include <gfx/renderer2.h>
#include <common/vk_initializers.h>
#include <core/components/components.h>
#include <core/systems/base_system.h>
#include "light.h"
#include "rhi/vk_image.h"

void ShadowRenderer::create(Renderer2& ren)
{
	shadow_map = GfxDevice::create_image(VkExtent2D{ 512,512 }, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	dir_light_sm_pipeline = pipeline::create_dir_light_pipeline(ren);

	GfxDevice::set_debug_name(shadow_map.image, "shadow_map");
}

void ShadowRenderer::render(VkCommandBuffer cmd, entt::registry& reg)
{
	DirectionalLight dl;
	dl.direction = glm::vec3(0.5f, 0.5f, 0.5f);

	VkClearValue clearValue;
	clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	VkRenderingAttachmentInfo depthAttachment = vkinit::depth_attachment_info(shadow_map.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	VkExtent2D _windowExtent = { 512, 512 };
	VkRenderingInfo render_info = vkinit::rendering_info(_windowExtent, nullptr, &depthAttachment);

	vkCmdBeginRendering(cmd, &render_info);

	//set dynamic viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = 512;
	viewport.height = 512;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = 512;
	scissor.extent.height = 512;

	vkCmdSetScissor(cmd, 0, 1, &scissor);

	VkDeviceSize offset = { 0 };

	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(dl.direction, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

	GPUDrawPushConstants pc;

	light_mtx = lightProjection * lightView;


	auto modelsToRender = reg.view<core_transform_component, core_mesh_component>();
	for (const auto&& [e, tc, mc] : modelsToRender.each())
	{
		tc.calculate_matrix();
		glm::mat4 model_matrix = tc.matrix;

		pc.model = lightProjection * lightView * model_matrix;

		for (auto& mesh : mc.model->meshes)
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, dir_light_sm_pipeline.pipeline);

			vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertex_buffer.buffer, &offset);
			vkCmdBindIndexBuffer(cmd, mesh.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdPushConstants(cmd, dir_light_sm_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GPUDrawPushConstants), &pc);

			vkCmdDrawIndexed(cmd, mesh.index_count, 1, 0, 0, 0);
		}
	};


	vkCmdEndRendering(cmd);

	//transition the shadow map image for sampling
	vkutil::transition_image(cmd, shadow_map.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, -1, true);
}

void ShadowRenderer::destroy()
{
	GfxDevice::destroy_image(shadow_map);
	GfxDevice::destroy_pipeline(dir_light_sm_pipeline.pipeline);
}
