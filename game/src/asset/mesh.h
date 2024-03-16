#pragma once

#include <vulkan/vulkan.h>

#include <openxr/openxr.h>
#include <common/GraphicsAPI.h>
#include <common/GraphicsAPI_Vulkan.h>
#include <common/xr_linear_algebra.h>

struct Mesh {
	VkBuffer index_buffer;
	VkBuffer vertex_buffer;
	uint32_t index_count;
	int material_index;
};

struct GPUModelConstant {
	XrMatrix4x4f model;
};

struct Material {
	GraphicsAPI::Image diff;
	GraphicsAPI::Image norm;
	//Image spec = nullptr;
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
	std::unordered_map<unsigned int, Material> materials;
};