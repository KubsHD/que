#pragma once

#include <core/types.h>
#include "base_asset.h"

enum class TextureFormat {
	// desktop
	TF_BC7,
	TF_BC5,
};

struct C_Texture {
	~C_Texture()
	{
		free(dds_blob);
	}

	asset_header header;
	
	uint32_t width;
	uint32_t height;

	uint8_t mip_levels;
	TextureFormat format;

	int blob_size;
	void* dds_blob;

	void serialize(std::ofstream& out);
	void read(std::vector<char> bytes);
};