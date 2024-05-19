#pragma once

#include <common/GraphicsAPI.h>
#include <common/GraphicsAPI_Vulkan.h>

#include <memory>

class Asset;

namespace pipeline {
	GraphicsAPI::Pipeline create_unlit_mesh_pipeline(GraphicsAPI_Vulkan& gapi, VkFormat drawImageFormat, VkFormat depthImageFormat);
}