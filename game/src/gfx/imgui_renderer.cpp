#include "pch.h"

#include "imgui_renderer.h"

#include <lib/imgui/imgui.h>
#include "rhi/vk_descriptor.h"
#include "renderer2.h"
#include <common/vk_initializers.h>

#define MAX_VTX 9999999
#define MAX_TEX 32

struct ImguiDrawData {
	Vec2 translation;
	Vec2 scale;
	int textureIndex;
};

struct ImguiVertex {
	Vec2 pos;
	Vec2 uv;
	unsigned int color;
};

void ImguiRenderer::init(Renderer2* r2)
{
	m_r2 = r2;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	io.Fonts->Build();
	io.DisplaySize = ImVec2(8, 8);
	io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	{
		DescriptorLayoutBuilder builder;

		builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		builder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		m_imgui_set_layout = builder.build(GfxDevice::device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		m_imgui_set = r2->global_descriptor_allocator.allocate(GfxDevice::device, m_imgui_set_layout);
	}

	{
		VkPipelineLayoutCreateInfo layout_info = vkinit::pipeline_layout_create_info();

		layout_info.pSetLayouts = &m_imgui_set_layout;
		layout_info.setLayoutCount = 1;

		VkPushConstantRange push_constant_range = {};
		push_constant_range.size = sizeof(ImguiDrawData);
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		layout_info.pPushConstantRanges = &push_constant_range;
		layout_info.pushConstantRangeCount = 1;

		vkCreatePipelineLayout(GfxDevice::device, &layout_info, nullptr, &m_imgui_pipeline.layout);

		PipelineBuilder builder;

		auto vs = PipelineBuilder::load_shader_module("shader/debug/imgui.vs_c", GfxDevice::device);
		auto ps = PipelineBuilder::load_shader_module("shader/debug/imgui.ps_c", GfxDevice::device);


		builder.set_shaders(vs, ps);
		builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
		builder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		builder.set_multisampling_none();
		builder.enable_blending_alphablend();
		builder.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);

		builder.set_color_attachment_format(r2->color_format);
		builder.set_depth_format(VK_FORMAT_D32_SFLOAT);

		builder.pipeline_layout = m_imgui_pipeline.layout;

		m_imgui_pipeline.pipeline = builder.build_pipeline(GfxDevice::device);
	}

	{
		imgui_vertices = GfxDevice::create_buffer(sizeof(ImguiVertex) * MAX_VTX, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		imgui_indices = GfxDevice::create_buffer(sizeof(uint16_t) * MAX_VTX, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	}

	{
		auto& io = ImGui::GetIO();

		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

		auto* pixels = static_cast<std::uint8_t*>(nullptr);
		int width = 0;
		int height = 0;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		imgui_font = GfxDevice::create_image(pixels, VkExtent2D{(std::uint32_t)width, (std::uint32_t)height }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

		io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(&imgui_font.view));
	}


	r2->main_deletion_queue.push_function([&]() {
		vkDestroyPipeline(GfxDevice::device, m_imgui_pipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(GfxDevice::device, m_imgui_pipeline.layout, nullptr);
		vkDestroyDescriptorSetLayout(GfxDevice::device, m_imgui_set_layout, nullptr);

		GfxDevice::destroy_buffer(imgui_vertices);
		GfxDevice::destroy_buffer(imgui_indices);

		GfxDevice::destroy_image(imgui_font);
	});
}

void ImguiRenderer::render(VkCommandBuffer cmd)
{
	const auto& dd = ImGui::GetDrawData();

	if (dd->CmdListsCount == 0)
		return;

	const auto clipOffset = dd->DisplayPos;
	const auto clipScale = dd->FramebufferScale;

	prepare_buffer(cmd);

	DescriptorWriter writer;

	writer.write_buffer(0, imgui_vertices.buffer, sizeof(ImguiVertex) * MAX_VTX, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	// todo: support more textures
	writer.write_image(1, imgui_font.view, m_r2->default_sampler_linear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	writer.update_set(GfxDevice::device, m_imgui_set);

	int vtxOffset = 0;
	int idxOffset = 0;

	for (int i = 0; i < dd->CmdListsCount; i++)
	{
		const ImDrawList* cmd_list = dd->CmdLists[i];

		vkCmdBindIndexBuffer(cmd, imgui_indices.buffer, 0, VK_INDEX_TYPE_UINT16);

		for (int cmdId = 0; cmdId < cmd_list->CmdBuffer.Size; cmdId++)
		{
			const auto& im_cmd = cmd_list->CmdBuffer[cmdId];
		
			if (im_cmd.ElemCount == 0)
				continue;

			auto clipMin = ImVec2(
				(im_cmd.ClipRect.x - clipOffset.x) * clipScale.x,
				(im_cmd.ClipRect.y - clipOffset.y) * clipScale.y);
			auto clipMax = ImVec2(
				(im_cmd.ClipRect.z - clipOffset.x) * clipScale.x,
				(im_cmd.ClipRect.w - clipOffset.y) * clipScale.y);
			clipMin.x = std::clamp(clipMin.x, 0.0f, (float)m_r2->depth_image.size.width);
			clipMax.x = std::clamp(clipMax.x, 0.0f, (float)m_r2->depth_image.size.width);
			clipMin.y = std::clamp(clipMin.y, 0.0f, (float)m_r2->depth_image.size.height);
			clipMax.y = std::clamp(clipMax.y, 0.0f, (float)m_r2->depth_image.size.height);

			if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) {
				continue;
			}

			const auto scissorX = static_cast<std::int32_t>(clipMin.x);
			const auto scissorY = static_cast<std::int32_t>(clipMin.y);
			const auto scissorWidth = static_cast<std::uint32_t>(clipMax.x - clipMin.x);
			const auto scissorHeight = static_cast<std::uint32_t>(clipMax.y - clipMin.y);
			const auto scissor = VkRect2D{
				{scissorX, scissorY},
				{scissorWidth, scissorHeight},
			};
			vkCmdSetScissor(cmd, 0, 1, &scissor);

			ImguiDrawData dd2;
			dd2.scale = Vec2(2.0f / dd->DisplaySize.x, 2.0f / dd->DisplaySize.y);
			dd2.translation = Vec2(-1.0f - dd->DisplayPos.x * dd2.scale.x, -1.0f - dd->DisplayPos.y * dd2.scale.y);
			
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_imgui_pipeline.pipeline);
			vkCmdPushConstants(cmd, m_imgui_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ImguiDrawData), &dd2);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_imgui_pipeline.layout, 0, 1, &m_imgui_set, 0, nullptr);

			vkCmdDrawIndexed(cmd, im_cmd.ElemCount, 1, im_cmd.IdxOffset + idxOffset, im_cmd.VtxOffset + vtxOffset, 0);
		}

		vtxOffset += cmd_list->VtxBuffer.Size;
		idxOffset += cmd_list->IdxBuffer.Size;
	}
}

void ImguiRenderer::prepare_buffer(VkCommandBuffer cmd)
{
	const auto& dd = ImGui::GetDrawData();

	if (dd->CmdListsCount == 0)
		return;

	int offset = 0;
	int idxOffset = 0;

	for (int i = 0; i < dd->CmdListsCount; i++)
	{
		const ImDrawList* cmd_list = dd->CmdLists[i];

		GfxDevice::upload_buffer(imgui_vertices, offset, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImguiVertex));
		GfxDevice::upload_buffer(imgui_indices, idxOffset, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(uint16_t));
		
		offset += cmd_list->VtxBuffer.Size * sizeof(ImguiVertex);
		idxOffset += cmd_list->IdxBuffer.Size * sizeof(uint16_t);
	}
}

