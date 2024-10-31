#pragma once

#include <core/types.h>

enum ShaderType {
	ST_VERTEX = 0,
	ST_PIXEL = 1,
	ST_COMPUTE = 2,

	ST_MAX = 99
};

struct ShaderEntry {
	uint32_t size;
	ShaderType type;
	char* blob;
};

struct ShaderMap {
	std::string name;
	uint32_t offset;
};

struct ShaderBundle {
	const char* header = "SBNDL";
	uint32_t shaders_count;

	ShaderMap* maps;

	ShaderEntry* shaders;
};