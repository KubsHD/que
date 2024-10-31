#pragma once

#include <core/types.h>
#include <gfx/rhi/gfx_device.h>
#include <core/audio.h>
#include "mesh.h"


class AssetManager {
public:
	static void Init(AudioSystem& asys);

	static std::vector<char> read_all_bytes(String path);
	static std::vector<char> read_all_bytes_raw(String path);


	static GPUImage load_image(String path, TextureType type);

	static std::shared_ptr<Sound> load_sound(String path);
	static Model load_model(Path path);
	static Model load_model_json(Path path);

	static nlohmann::json read_json(String path);
private:
	static std::unordered_map<std::string, std::shared_ptr<Sound>> m_sound_cache;
	static AudioSystem* m_audio_system_reference;
};
