#pragma once

#include <vulkan/vulkan.h>

struct Mesh {
	VkBuffer* index_buffer;
	VkBuffer* vertex_buffer;
	uint32_t index_count;
};

struct Vertex {
	float x;
	float y;
	float z;

	float nx;
	float ny;
	float nz;

	float u;
	float v;
};

struct Model {
	std::vector<Mesh> meshes;
};