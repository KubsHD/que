#pragma once

#include <core/types.h>
#include "base_asset.h"


enum ShaderType {
	ST_VERTEX = 0,
	ST_PIXEL = 1,
	ST_COMPUTE = 2,

	ST_MAX = 99
};

// individual shader

struct C_Shader {
	asset_header header;
	ShaderType type;
	uint32_t size;
	void* blob;
	void serialize(std::ofstream& out);
};

// bundle


struct ShaderEntry {
	ShaderType type;
	uint32_t size; // size of the blob
	std::byte* blob;
};

struct ShaderMap {
	const char* name;
	uint32_t name_length;
	uint32_t offset;
};

struct ShaderBundle {
	const char* header = "SBNDL";
	uint32_t shaders_count;
	// add padding
	char padding[5];


	ShaderMap* maps;
	ShaderEntry* shaders;

	void init()
	{
		shaders_count = 0;
		maps = nullptr;
		shaders = nullptr;
	}

	void add_shader(const char* name, ShaderType type, const char* blob, uint32_t size);
	void serialize(std::ofstream& out);
};