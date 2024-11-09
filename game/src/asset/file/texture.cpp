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

void C_Texture::read(std::vector<char> bytes)
{
	size_t offset = 5;
	memcpy(&header.version, bytes.data() + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(header.hash, bytes.data() + offset, 32);
	offset += 32;
	memcpy(header.padding, bytes.data() + offset, 7);
	offset += 7;
	memcpy(&width, bytes.data() + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&height, bytes.data() + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&mip_levels, bytes.data() + offset, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&format, bytes.data() + offset, sizeof(TextureFormat));
	offset += sizeof(TextureFormat);
	memcpy(&blob_size, bytes.data() + offset, sizeof(int));
	offset += sizeof(int);
	dds_blob = malloc(blob_size);
	memcpy(dds_blob, bytes.data() + offset, blob_size);
}
