#pragma once

#include <vulkan/vulkan.h>
#include <array>

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

	static VkVertexInputBindingDescription get_binding_description() {
		VkVertexInputBindingDescription binding_description = {};
		binding_description.binding = 0;
		binding_description.stride = sizeof(Vertex);
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding_description;
	}

	static std::array<VkVertexInputAttributeDescription, 5> get_attributes_descriptions() {
		std::array<VkVertexInputAttributeDescription, 5> attribute_descriptions = {};
		// Position
		attribute_descriptions[0].binding = 0;
		attribute_descriptions[0].location = 0;
		attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[0].offset = offsetof(Vertex, x);
		// Normal
		attribute_descriptions[1].binding = 0;
		attribute_descriptions[1].location = 1;
		attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[1].offset = offsetof(Vertex, nx);
		// Tangent
		attribute_descriptions[2].binding = 0;
		attribute_descriptions[2].location = 2;
		attribute_descriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[2].offset = offsetof(Vertex, tx);
		// Bitangent
		attribute_descriptions[3].binding = 0;
		attribute_descriptions[3].location = 3;
		attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[3].offset = offsetof(Vertex, btx);
		// UV
		attribute_descriptions[4].binding = 0;
		attribute_descriptions[4].location = 4;
		attribute_descriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_descriptions[4].offset = offsetof(Vertex, u);
		return attribute_descriptions;
	}
};