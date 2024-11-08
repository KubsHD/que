#pragma once

#include <core/types.h>

void compile_shaders(std::vector<fs::path> paths);


class ResourceCompiler {
public:
	static void Compile(fs::path source_data_path, fs::path output_dir);
};