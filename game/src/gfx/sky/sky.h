#pragma once

#include <core/types.h>
#include <gfx/rhi/gfx_device.h>
#include <gfx/mat/mat_sky.h>

#include <asset/mesh.h>

#include "sky_types.h"

struct Model;
class Renderer2;

namespace gfx {
	struct Sky {
		GPUImage skyImage;
		// cube probably
		GPUImage skyCubemap;
		GPUImage skyIrradiance;

		Model skybox_cube;

		MAT_Sky mat_sky;
		MaterialInstance mat_sky_instance;

		void draw(Renderer2& ren, VkCommandBuffer cmd);
		void create(Renderer2& gapi, String hdriPath, String cube);
		void clear(VkDevice device);
	};

}
