#pragma once


#include <gfx/pipeline/builder.h>
#include <memory>

#include "sky_types.h"

class Renderer2;

namespace pipeline {
	GPUPipeline create_sky_irradiance_pipeline(Renderer2& gapi);
}