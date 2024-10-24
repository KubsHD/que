#include "pch.h"

#include "sky_pipeline.h"

#include <asset/mesh.h>
#include <core/asset.h>

namespace pipeline
{
	GraphicsAPI::Pipeline create_sky_pipeline(GraphicsAPI_Vulkan& gapi, VkFormat drawImageFormat, VkFormat depthImageFormat)
	{
		std::vector<char> vertexSource = Asset::Instance->read_all_bytes("shader/sky.vert.spv");
		auto vs = gapi.CreateShader({ GraphicsAPI::ShaderCreateInfo::Type::VERTEX, vertexSource.data(), vertexSource.size() });

		std::vector<char> fragmentSource = Asset::Instance->read_all_bytes("shader/sky.frag.spv");
		auto ps = gapi.CreateShader({ GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT, fragmentSource.data(), fragmentSource.size() });

		VkPushConstantRange range;
		range.offset = 0;
		range.size = sizeof(GPUModelConstant);
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		GraphicsAPI::PipelineCreateInfo pipelineCI;

		pipelineCI.shaders = { vs, ps };
		pipelineCI.vertexInputState.attributes = {
			{0, 0, GraphicsAPI::VertexType::VEC3, 0, "POSITION"},
			{1, 0, GraphicsAPI::VertexType::VEC3, offsetof(Vertex, nx), "NORMAL"},
			{2, 0, GraphicsAPI::VertexType::VEC3, offsetof(Vertex, tx), "TANGENT"},
			{3, 0, GraphicsAPI::VertexType::VEC3, offsetof(Vertex, btx), "BITANGENT"},
			{4, 0, GraphicsAPI::VertexType::VEC2, offsetof(Vertex, u), "TEXCOORD"}
		};
		// stride VVVV ie. sizeof(Vertex)
		pipelineCI.vertexInputState.bindings = { {0, 0, 14 * sizeof(float)} };
		pipelineCI.inputAssemblyState = { GraphicsAPI::PrimitiveTopology::TRIANGLE_LIST, false };
		pipelineCI.rasterisationState = { false, false, GraphicsAPI::PolygonMode::FILL,
										GraphicsAPI::CullMode::NONE, GraphicsAPI::FrontFace::CLOCKWISE,
										false, 0.0f, 0.0f, 0.0f, 1.0f };
		pipelineCI.multisampleState = { 1, false, 1.0f, 0xFFFFFFFF, false, false };
		pipelineCI.depthStencilState = { true, false, GraphicsAPI::CompareOp::LESS_OR_EQUAL, false, false, {}, {}, 0.0f, 1.0f };
		pipelineCI.colorBlendState = { false, GraphicsAPI::LogicOp::NO_OP, {{true, GraphicsAPI::BlendFactor::SRC_ALPHA, GraphicsAPI::BlendFactor::ONE_MINUS_SRC_ALPHA, GraphicsAPI::BlendOp::ADD, GraphicsAPI::BlendFactor::ONE, GraphicsAPI::BlendFactor::ZERO, GraphicsAPI::BlendOp::ADD, (GraphicsAPI::ColorComponentBit)15}}, {0.0f, 0.0f, 0.0f, 0.0f} };
		pipelineCI.colorFormats = { drawImageFormat };
		pipelineCI.depthFormat = depthImageFormat;
		pipelineCI.layout = { {0, 0, nullptr, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
							  {0, 1, nullptr, nullptr, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT} };
		pipelineCI.pushConstantRange = range;

		return gapi.CreatePipeline(pipelineCI);
	}
}

