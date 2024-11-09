#include "pch.h"

#include "util.h"
#include "file/base_asset.h"
#include <core/profiler.h>
#include <lib/md5.h>

bool check_if_requires_recompilation(std::string& source_file_hash, fs::path compiled_file_path)
{
	QUE_PROFILE;

	if (!fs::exists(compiled_file_path))
		return true;

	std::ifstream compiled_file(compiled_file_path, std::ios::binary);

	asset_header header;
	header.read(compiled_file);

	return header.hash != source_file_hash;
}

std::string get_file_hash(std::string data)
{
	QUE_PROFILE;

	Chocobo1::MD5 md5;
	md5.addData(data.data(), data.size());
	md5.finalize();
	return md5.toString();
}

