#include "pch.h"

#include "sky_compiler.h"

#include <cuttlefish/Texture.h>
#include "nvtt_blob_output.h"
#include <asset/file/texture.h>
#include <lib/md5.h>
#include <fstream>
#include <asset/util.h>
#include <core/profiler.h>


void compile_skies(fs::path source_data_path, std::vector<fs::path> paths, fs::path output_path)
{
	QUE_PROFILE;

	fs::path cache_path = output_path;

	for (auto& path : paths)
	{
		std::ifstream asset(path, std::ios::binary);

		std::string str;
		str.resize(fs::file_size(path));
		asset.read(str.data(), str.size());

		fs::path asset_relative_path = path.lexically_relative(source_data_path);
		asset_relative_path.replace_extension("sky_c");

		auto hash = get_file_hash(str);

		if (!check_if_requires_recompilation(hash, cache_path / asset_relative_path))
			continue;

		{
			QUE_PROFILE_SECTION("Sky Processing");

			cuttlefish::Image image;
			image.load(asset_relative_path.string().c_str());

			cuttlefish::Texture texture;

			texture.setImage(image);
			texture.convert(cuttlefish::Texture::Format::BC6H, cuttlefish::Texture::Type::Float);


//todo

			C_Texture ct{};

			std::strcpy(ct.header.hash, hash.c_str());

			// ct.dds_blob = malloc(blob.size);
			// ct.blob_size = blob.size;
			// memcpy(ct.dds_blob, blob.data, blob.size);

			std::ofstream out(cache_path / asset_relative_path, std::ios::binary);
			ct.serialize(out);
		}
	}
}

