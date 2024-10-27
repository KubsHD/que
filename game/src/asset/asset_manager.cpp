#include "pch.h"

#include "asset_manager.h"

std::vector<char> AssetManager::read_all_bytes(String path)
{

#if defined(__ANDROID__)
	AAsset* file = AAssetManager_open(m_android_asset_manager, path.c_str(), AASSET_MODE_BUFFER);
	size_t fileLength = AAsset_getLength(file);
	std::vector<char> binary(fileLength);
	AAsset_read(file, (void*)binary.data(), fileLength);
	AAsset_close(file);
	return binary;
#else

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
