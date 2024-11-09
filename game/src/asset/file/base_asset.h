#pragma once

#include <cstdint>

struct asset_header {
	const char magic[5] = "QUEB";
	uint32_t version = 1;
	char hash[32];
	char padding[7];

	void read(std::istream& in)
	{
		in.read((char*)magic, 5);
		in.read((char*)&version, sizeof(version));
		in.read(hash, 32);
		in.read(padding, 7);
	}
};