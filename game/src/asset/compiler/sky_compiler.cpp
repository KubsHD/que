#include "pch.h"

#include "sky_compiler.h"

#include <lib/dds-ktx.h>
#include <nvtt/nvtt.h>
#include "nvtt_blob_output.h"
#include <asset/file/texture.h>
#include <lib/md5.h>
#include <fstream>
#include <asset/util.h>
#include <core/profiler.h>

void compile_skies(fs::path source_data_path, std::vector<fs::path> paths)
{
	QUE_PROFILE;

	fs::path cache_path = ".cache";

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

			nvtt::Context context;

			NvttBlob blob;

			nvtt::CompressionOptions compression_options;
			compression_options.setFormat(nvtt::Format_DXT1);

			nvtt::OutputOptions output_options;
			output_options.setOutputHandler(&blob);

			nvtt::Surface surface;
			surface.load(path.string().c_str());

			nvtt::CubeSurface cube_surface;
			cube_surface.fold(surface, nvtt::CubeLayout_HorizontalCross);

			context.outputHeader(cube_surface, 1, compression_options, output_options);
			context.compress(cube_surface, 0, compression_options, output_options);

			C_Texture ct{};

			std::strcpy(ct.header.hash, hash.c_str());

			ct.dds_blob = malloc(blob.size);
			ct.blob_size = blob.size;
			memcpy(ct.dds_blob, blob.data, blob.size);



			std::ofstream out(cache_path / asset_relative_path, std::ios::binary);
			ct.serialize(out);
		}
	}
}

