#pragma once

#include <common/GraphicsAPI_Vulkan.h>
#include <core/types.h>

struct Model;

namespace gfx {
	struct Sky {
		GraphicsAPI::Image skyImage;
		GraphicsAPI::Image skyCubemap;
		// cube probably
	};

	namespace sky {
		Sky create_sky(GraphicsAPI_Vulkan& gapi, String hdriPath, Model cube, GraphicsAPI::Pipeline sky_pipeline);
	}
}
