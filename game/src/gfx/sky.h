#pragma once

#include <core/types.h>
#include <gfx/rhi/gfx_device.h>

struct Model;
class Renderer2;

namespace gfx {
	struct Sky {
		GPUPipeline sky_render_pipeline;
		GPUPipeline sky_cube_render_pipeline;

		GPUImage skyImage;
		// cube probably
		GPUImage skyCubemap;
		GPUImage skyIrradiance;
	};

	namespace sky {
		Sky create_sky(Renderer2& gapi, String hdriPath, String cube);
	}
}
