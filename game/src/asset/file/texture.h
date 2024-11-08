#pragma once

#include <core/types.h>
#include "base_asset.h"

enum class TextureFormat {
	TF_BC7,
	TF_BC7n,
};

struct C_Texture {
	asset_header header;
	
	uint32_t width;
	uint32_t height;

	uint8_t mip_levels;
	TextureFormat format;

	int blob_size;
	void* dds_blob;

	void serialize(std::ofstream& out);
};