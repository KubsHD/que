#pragma once

#include <memory>

#include <gfx/rhi/gfx_device.h>

class Renderer2;
class AssetSystem;

namespace pipeline {
	GPUPipeline create_sky_cube_render_pipeline(Renderer2& gapi);
}