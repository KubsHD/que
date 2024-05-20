#pragma once

#include <common/GraphicsAPI_Vulkan.h>
#include <core/types.h>

struct Model;

namespace gfx {
	struct Sky {
		GraphicsAPI::Image skyImage;
		// cube probably
		GraphicsAPI::Image skyCubemap;

		GraphicsAPI::Image skyIrradiance;
	};

	namespace sky {
		Sky create_sky(GraphicsAPI_Vulkan& gapi, String hdriPath, Model cube, GraphicsAPI::Pipeline sky_render_pipeline);
	}
}
