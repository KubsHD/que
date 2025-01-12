#pragma once

#include <core/types.h>


namespace gfx {

	struct DirectionalLight {
		Vec3 direction;
		Vec3 color;
		float intensity;
	};

	struct PointLight {
		Vec3 position;
		Vec3 color;
		float intensity;
		float range;
	};

	struct SpotLight {
		Vec3 position;
		Vec3 direction;
		Vec3 color;
		float intensity;
		float range;
		float angle;
	};
}