#pragma once

#include <core/types.h>
#include <gfx/rhi/gfx_device.h>
#include <core/audio.h>
#include <gfx/mesh.h>
#include <assimp/scene.h>

class Renderer2;

class AssetManager {
public:
	static void PreInit();
	static void Init(AudioSystem& asys, Renderer2& ren);
	static void Destroy();

	static std::vector<char> read_all_bytes(String path);
	static std::vector<char> read_all_bytes_raw(String path);

	static GPUImage load_texture(String path, TextureType type);
	static GPUImage load_texture_c(String path, TextureType type);

	static std::shared_ptr<Sound> load_sound(String path);
	static Model load_model(Path path);
	static Model load_model_json(Path path);

	static nlohmann::json read_json(String path);

	static fs::path get_asset_dir();
private:
	static GPUImage AssetManager::try_to_load_texture_type(const aiScene* scene, aiMaterial* material, aiTextureType type, String root_path);
	static std::unordered_map<std::string, std::shared_ptr<Sound>> m_sound_cache;
	static AudioSystem* m_audio_system_reference;
	static Renderer2* m_renderer_reference;
};
