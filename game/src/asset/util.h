#pragma once

#include <core/types.h>

bool check_if_requires_recompilation(std::string& source_file_hash, fs::path compiled_file_path);

std::string get_file_hash(std::string data);