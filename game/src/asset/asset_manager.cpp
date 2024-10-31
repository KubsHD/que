#include "pch.h"

#include "asset_manager.h"
#include <common/DebugOutput.h>

fs::path root_path;

std::string cache_path;

void AssetManager::Init()
{
	cache_path = ".cache/";

#if _DEBUG

	if (!fs::exists(cache_path))
		fs::create_directory(cache_path);

	root_path = fs::current_path();

	auto test_path = root_path;
	test_path.append(".gitignore");

	// iterate up until we are in directory with .gitignore file
	while (!fs::exists(test_path))
	{
		root_path = root_path.parent_path();
		
		test_path = root_path;
		test_path.append(".gitignore");
	}

	root_path.append("game\\data");

	LOG_INFO("Root path: " << root_path.string().c_str());

#else
	root_path = "data/";
#endif
}

std::vector<char> AssetManager::read_all_bytes(String path)
{
	fs::path real_path = root_path / path;
	return read_all_bytes_raw(real_path.string());
}

std::vector<char> AssetManager::read_all_bytes_raw(String path)
{
#if defined(__ANDROID__)
	AAsset* file = AAssetManager_open(m_android_asset_manager, path.c_str(), AASSET_MODE_BUFFER);
	size_t fileLength = AAsset_getLength(file);
	std::vector<char> binary(fileLength);
	AAsset_read(file, (void*)binary.data(), fileLength);
	AAsset_close(file);
	return binary;
#else
	if (!fs::exists(path))
		abort();

	std::ifstream stream(path, std::fstream::in | std::fstream::binary | std::fstream::ate);
	if (!stream.is_open()) {
		std::cout << "Could not read file " << path.c_str() << ". File does not exist." << std::endl;
		return {};
	}
	std::streamoff size = stream.tellg();
	std::vector<char> output(static_cast<size_t>(size));
	stream.seekg(0, std::fstream::beg);
	stream.read(output.data(), size);
	stream.close();
	return output;
#endif
}
