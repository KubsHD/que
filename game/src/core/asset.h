#pragma once

#include <core/types.h>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

#include <vector>
#include <asset/mesh.h>

#include <common/GraphicsAPI.h>

class GraphicsAPI_Vulkan;

class Asset {
public:
	Asset() = default;
	Asset(void* android_ass);
	~Asset();

	std::vector<char> read_all_bytes(String path);
	
	static void load_texture_from_file(GraphicsAPI_Vulkan& gapi, String path);
	static Mesh load_mesh(GraphicsAPI_Vulkan& gapi, String path);

	static Model load_model(GraphicsAPI_Vulkan& gapi, String path);

	static GraphicsAPI::Image load_image(GraphicsAPI_Vulkan& gapi, String path, bool isHdri = false);
private:
#if defined(__ANDROID__)
	AAssetManager* m_android_asset_manager;
#endif
};