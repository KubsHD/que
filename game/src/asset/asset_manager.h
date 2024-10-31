#pragma once

#include <core/types.h>
#include <gfx/rhi/gfx_device.h>
#include <core/audio.h>
#include "mesh.h"

class AssetManager {
public:
	static void Init();

	static std::vector<char> read_all_bytes(String path);
	static GPUImage load_image(String path, TextureType type);

	static std::shared_ptr<Sound> load_sound(String path);
	static Model load_model(Path path);
	static Model load_model_json(Path path);
};
