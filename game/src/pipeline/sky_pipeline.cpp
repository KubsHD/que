#include "sky_pipeline.h"

#include <asset/mesh.h>
#include <core/asset.h>

namespace pipeline
{
	VkPipeline create_sky_pipeline(GraphicsAPI_Vulkan& gapi, std::shared_ptr<Asset> asset_mgr, VkFormat drawImageFormat, VkFormat depthImageFormat)
	{
		std::vector<char> vertexSource = asset_mgr->read_all_bytes("data/shader/sky.vert.spv");
		auto vs = gapi.CreateShader({ GraphicsAPI::ShaderCreateInfo::Type::VERTEX, vertexSource.data(), vertexSource.size() });

		std::vector<char> fragmentSource = asset_mgr->read_all_bytes("data/shader/sky.frag.spv");
		auto ps = gapi.CreateShader({ GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT, fragmentSource.data(), fragmentSource.size() });

		GraphicsAPI::PipelineCreateInfo pipelineCI;

		pipelineCI.shaders = { vs, ps };
		pipelineCI.vertexInputState.attributes = {
			{0, 0, GraphicsAPI::VertexType::VEC3, 0, "POSITION"},
			{1, 0, GraphicsAPI::VertexType::VEC3, offsetof(Vertex, nx), "NORMAL"},
			{2, 0, GraphicsAPI::VertexType::VEC2, offsetof(Vertex, u), "TEXCOORD"}
		};
		// stride VVVV ie. sizeof(Vertex)
		pipelineCI.vertexInputState.bindings = { {0, 0, 8 * sizeof(float)} };
		pipelineCI.inputAssemblyState = { GraphicsAPI::PrimitiveTopology::TRIANGLE_LIST, false };
		pipelineCI.rasterisationState = { false, false, GraphicsAPI::PolygonMode::FILL,
										GraphicsAPI::CullMode::BACK, GraphicsAPI::FrontFace::CLOCKWISE,
										false, 0.0f, 0.0f, 0.0f, 1.0f };
		pipelineCI.multisampleState = { 1, false, 1.0f, 0xFFFFFFFF, false, false };
		pipelineCI.depthStencilState = { true, true, GraphicsAPI::CompareOp::LESS_OR_EQUAL, false, false, {}, {}, 0.0f, 1.0f };
		pipelineCI.colorBlendState = { false, GraphicsAPI::LogicOp::NO_OP, {{true, GraphicsAPI::BlendFactor::SRC_ALPHA, GraphicsAPI::BlendFactor::ONE_MINUS_SRC_ALPHA, GraphicsAPI::BlendOp::ADD, GraphicsAPI::BlendFactor::ONE, GraphicsAPI::BlendFactor::ZERO, GraphicsAPI::BlendOp::ADD, (GraphicsAPI::ColorComponentBit)15}}, {0.0f, 0.0f, 0.0f, 0.0f} };
		pipelineCI.colorFormats = { drawImageFormat };
		pipelineCI.depthFormat = depthImageFormat;
		pipelineCI.layout = { {0, nullptr, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
							  {1, nullptr, nullptr, GraphicsAPI::DescriptorInfo::Type::IMAGE, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT} };

		return gapi.CreatePipeline(pipelineCI);

	}
}

