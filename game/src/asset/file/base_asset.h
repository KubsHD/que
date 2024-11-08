#pragma once

#include <cstdint>

struct asset_header {
	const char magic[5] = "QUEB";
	uint32_t version = 1;
	char hash[32];
	char padding[7];
};