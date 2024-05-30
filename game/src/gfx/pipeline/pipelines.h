#pragma once

enum class PipelineType {
	LIT,
	UNLIT,
	LIGHT,
	SKY,
	SKY_CUBE, // move to compute
	SKY_IRRADIANCE // move to compute
};