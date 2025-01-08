#pragma once

#include <core/types.h>

#include <vulkan/vulkan.h>

struct GPUPipeline;

struct MaterialInstance {
	GPUPipeline* pipeline;
	VkDescriptorSet material_set;
};


class PipelineBuilder {
public:
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

	VkPipelineInputAssemblyStateCreateInfo input_assembly;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState color_blend_attachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineLayout pipeline_layout;
	VkPipelineDepthStencilStateCreateInfo depth_stencil;
	VkPipelineRenderingCreateInfo render_info;
	VkFormat color_attachment_format;
	VkPipelineVertexInputStateCreateInfo vertex_input_info;

	PipelineBuilder() { clear(); }

	void clear();

	void set_shaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader, VkShaderModule geometry_shader = nullptr);
	void set_input_topology(VkPrimitiveTopology topology);
	void set_polygon_mode(VkPolygonMode mode);
	void set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);
	void set_multisampling_none();
	void disable_blending();
	void enable_blending_additive();
	void enable_blending_alphablend();

	void set_color_attachment_format(VkFormat format);
	void set_depth_format(VkFormat format);
	void disable_depthtest();
	void enable_depthtest(bool depthWriteEnable, VkCompareOp op);

	VkPipeline build_pipeline(VkDevice device);

	static VkShaderModule load_shader_module(const char* filename, VkDevice device);
};