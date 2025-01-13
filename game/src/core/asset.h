#pragma once

#include <core/types.h>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

#include <vector>
#include <gfx/mesh.h>
#include <core/audio.h>

#include <common/GraphicsAPI.h>
#include <assimp/material.h>

class AudioSystem;
class GraphicsAPI_Vulkan;
struct aiScene;
struct aiMaterial;



class AssetSystem {
public:
	static AssetSystem* Instance;

	AssetSystem(AudioSystem* audio_sys, GraphicsAPI_Vulkan* api);
	AssetSystem(void* android_ass, AudioSystem* audio_sys, GraphicsAPI_Vulkan* api);
	~AssetSystem();

	std::vector<char> read_all_bytes(String path);

	static GraphicsAPI::Image load_image(String path, TextureType type);
	
	static std::shared_ptr<Sound> load_sound(String path);

	static Model load_model(Path path);


	static Model load_model_json(Path path);

	static nlohmann::json read_json(String path);
private:
	static std::unordered_map<std::string, std::shared_ptr<Sound>> m_sound_cache;

	static GraphicsAPI::Image try_to_load_texture_type(const aiScene* scene, aiMaterial* material, aiTextureType type, String root_path);

	AudioSystem* m_audio_system_reference;

	static GraphicsAPI_Vulkan* m_api;

#if defined(__ANDROID__)
	AAssetManager* m_android_asset_manager;
#endif
};