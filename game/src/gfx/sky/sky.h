#pragma once

#include <core/types.h>
#include <gfx/rhi/gfx_device.h>

#include "sky_types.h"

struct Model;
class Renderer2;

namespace gfx {
	struct Sky {
		SkyIntermiedatePipeline sky_render_pipeline{};
		SkyIntermiedatePipeline sky_cube_render_pipeline{};

		GPUImage skyImage;
		// cube probably
		GPUImage skyCubemap;
		GPUImage skyIrradiance;
	};

	Sky create_sky(Renderer2& gapi, String hdriPath, String cube);
}
