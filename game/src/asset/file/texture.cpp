#include "pch.h"

#include "texture.h"

void C_Texture::serialize(std::ofstream& out)
{
	out.write(header.magic, 5);
	out.write((char*)&header.version, sizeof(uint32_t));
	out.write(header.hash, 32);
	out.write(header.padding, 7);

	out.write((char*)&width, sizeof(uint32_t));
	out.write((char*)&height, sizeof(uint32_t));
	
	out.write((char*)&mip_levels, sizeof(uint8_t));
	out.write((char*)&format, sizeof(TextureFormat));

	out.write((char*)&blob_size, sizeof(int));
	out.write((char*)dds_blob, blob_size);
}
