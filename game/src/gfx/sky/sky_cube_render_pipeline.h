#pragma once

#include <memory>

#include <gfx/rhi/gfx_device.h>
#include "sky_types.h"


class Renderer2;
class AssetSystem;

namespace pipeline {
	SkyIntermiedatePipeline create_sky_cube_render_pipeline(Renderer2& gapi);
}