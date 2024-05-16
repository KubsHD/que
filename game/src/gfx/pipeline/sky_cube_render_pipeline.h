#pragma once

#include <common/GraphicsAPI.h>
#include <common/GraphicsAPI_Vulkan.h>

#include <memory>

class Asset;

namespace pipeline {
	GraphicsAPI::Pipeline create_sky_cube_render_pipeline(GraphicsAPI_Vulkan& gapi);
}