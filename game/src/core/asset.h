#pragma once

#include <core/types.h>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

#include <vector>
#include <asset/mesh.h>
#include <core/audio.h>

#include <common/GraphicsAPI.h>
#include <assimp/material.h>

class AudioSystem;
class GraphicsAPI_Vulkan;
struct aiScene;
struct aiMaterial;

enum TextureType {
	TT_DIFFUSE = 0,
	TT_NORMAL = 1,
	TT_HDRI = 2
};

class Asset {
public:
	static Asset* Instance;

	Asset(AudioSystem* audio_sys);
	Asset(void* android_ass, AudioSystem* audio_sys);
	~Asset();

	std::vector<char> read_all_bytes(String path);

	static GraphicsAPI::Image load_image(GraphicsAPI_Vulkan& gapi, String path, TextureType type);
	
	static std::shared_ptr<Sound> load_sound(String path);
	static Model load_model(GraphicsAPI_Vulkan& gapi, Path path);
	static Model load_model_json(GraphicsAPI_Vulkan& gapi, Path path);

	static nlohmann::json read_json(String path);
private:
	static std::unordered_map<std::string, std::shared_ptr<Sound>> m_sound_cache;

	static GraphicsAPI::Image try_to_load_texture_type(GraphicsAPI_Vulkan& gapi, const aiScene* scene, aiMaterial* material, aiTextureType type, String root_path);

	AudioSystem* m_audio_system_reference;

#if defined(__ANDROID__)
	AAssetManager* m_android_asset_manager;
#endif
};