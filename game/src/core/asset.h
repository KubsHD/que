#pragma once

#include <core/types.h>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

#include <vector>

class Asset {
public:
	Asset() = default;
	Asset(void* android_ass);
	~Asset();

	std::vector<char> read_all_bytes(String path);

private:
#if defined(__ANDROID__)
	AAssetManager* m_android_asset_manager;
#endif
};