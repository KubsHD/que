#include "pch.h"

#include "sky_pipeline.h"

#include <asset/mesh.h>
#include <core/profiler.h>

#include <gfx/renderer2.h>

namespace pipeline {

	GPUPipeline create_sky_cube_render_pipeline(Renderer2& gapi)
	{
		QUE_PROFILE;

		GPUPipeline pip;

		auto vs = PipelineBuilder::load_shader_module("shader/equi2cube.vs_c", GfxDevice::device);
		auto ps = PipelineBuilder::load_shader_module("shader/equi2cube.ps_c", GfxDevice::device);

		VkPushConstantRange range;
		range.offset = 0;
		range.size = sizeof(GPUEqui2CubeConstant);
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;



		

		return pip;
	}
}