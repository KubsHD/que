#pragma once


#include "sky_types.h"

class Renderer2;
class AssetSystem;

namespace pipeline {
	SkyIntermiedatePipeline create_sky_pipeline(Renderer2& ren);
}