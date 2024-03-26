#pragma once

#include <openxr/openxr.h>
#include <asset/mesh.h>

struct transform_component {
	XrVector3f position;
};

struct mesh_component {
	Model model;
};