#pragma once

#include "pch.h"

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

struct GPUEqui2CubeConstant {
	glm::mat4 viewProj;
};

struct Material {
	std::string name;

	GraphicsAPI::Image diff;
	GraphicsAPI::Image norm;
	GraphicsAPI::Image orm;

	glm::vec3 color;
};

struct Vertex {
	float x;
	float y;
	float z;

	float nx;
	float ny;
	float nz;

	float tx;
	float ty;
	float tz;

	float btx;
	float bty;
	float btz;

	float u;
	float v;
};

struct Model {
	std::string name;
	std::vector<Mesh> meshes;
	std::unordered_map<unsigned int, Material> materials;
};