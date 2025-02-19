#pragma once

#include "pch.h"

#include <common/xr_linear_algebra.h>
#include <gfx/pipeline/builder.h>
#include <gfx/vertex.h>
#include <gfx/rhi/gfx_device.h>

struct GPUModelConstant {
	XrMatrix4x4f model;
};

struct GPUEqui2CubeConstant {
	glm::mat4 viewProj;
};


struct ColorData {
	glm::vec3 color;
};


struct Mesh {
	std::vector<Vertex2> vertices;
	std::vector<uint32_t> indices;

	GPUBuffer index_buffer;
	GPUBuffer vertex_buffer;
	uint32_t index_count;
	int material_index;
};

struct Model {
	std::string name;
	std::vector<Mesh> meshes;
	std::unordered_map<unsigned int, MaterialInstance> materials2;
};

