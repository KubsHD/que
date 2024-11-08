#include "pch.h"

#include "shader_bundle.h"

#include <dxc/dxcapi.h>

void ShaderBundle::add_shader(const char* name, ShaderType type, const char* blob, uint32_t size)
{
	shaders_count++;

	maps = (ShaderMap*)realloc(maps, sizeof(ShaderMap) * shaders_count);

	maps[shaders_count - 1].name_length = strlen(name);

	maps[shaders_count - 1].name = (char*)malloc(maps[shaders_count - 1].name_length * sizeof(char));
	strcpy((char*)maps[shaders_count - 1].name, name);

	int shadersCurrentSize = 0;
	for (int i = 0; i < shaders_count - 1; i++)
	{
		shadersCurrentSize += shaders[i].size * sizeof(std::byte) + sizeof(ShaderEntry::type) + sizeof(ShaderEntry::size);
	}

	shaders = (ShaderEntry*)realloc(shaders, shadersCurrentSize + (size * sizeof(std::byte) + sizeof(ShaderEntry::type) + sizeof(ShaderEntry::size)));

	shaders[shaders_count - 1].type = type;
	shaders[shaders_count - 1].size = size;

	shaders[shaders_count - 1].blob = (std::byte*)malloc(size * sizeof(std::byte));
	memcpy((std::byte*)shaders[shaders_count - 1].blob, blob, size);
	
	maps[shaders_count - 1].offset = shadersCurrentSize;
}

void ShaderBundle::serialize(std::ofstream& out)
{
	out.write(header, 5);
	out.write((char*)&shaders_count, sizeof(uint32_t));
	out.write(padding, 5);
	for (int i = 0; i < shaders_count; i++)
	{
		out.write((char*)&maps[i].name_length, sizeof(uint32_t));
		out.write(maps[i].name, maps[i].name_length);
		out.write((char*)&maps[i].offset, sizeof(uint32_t));
	}
	for (int i = 0; i < shaders_count; i++)
	{
		out.write((char*)&shaders[i].type, sizeof(ShaderType));
		out.write((char*)&shaders[i].size, sizeof(uint32_t));
		out.write((char*)shaders[i].blob, shaders[i].size);
	}
}

void C_Shader::serialize(std::ofstream& out)
{
	out.write(header.magic, 5);
	out.write((char*)&header.version, sizeof(uint32_t));
	out.write(header.hash, 32);
	out.write(header.padding, 7);

	out.write((char*)&type, sizeof(ShaderType));
	out.write((char*)&size, sizeof(uint32_t));
	out.write((char*)blob, size);
}
