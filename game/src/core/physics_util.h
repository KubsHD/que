#pragma once

#include <core/types.h>
#include <gfx/mesh.h>

namespace core {
	namespace physics {
		JPH::RefConst<JPH::Shape> create_mesh_shape(Mesh m, Vec3 scale = Vec3(1.0f));
		JPH::RefConst<JPH::Shape> create_convex_shape(Mesh m);
		JPH::RefConst<JPH::Shape> load_from_file(String path);
	}
}