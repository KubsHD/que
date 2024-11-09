#include "pch.h"

#include "resource_compiler.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <Shlwapi.h>
#include <core/types.h>

#include <fmod/win/fsbank/inc/fsbank.h>
#include <dxc/dxcapi.h>
#include "file/shader_bundle.h"
#include <wrl/client.h>
#include <lib/md5.h>
#include <codecvt>
#include <asset/file/shader_bundle.h>
#include <asset/file/texture.h>
#include <asset/resource_compiler.h>
#include <asset/asset_manager.h>

#include <nvtt/nvtt.h>
#include <core/profiler.h>
#include "compiler/sky_compiler.h"
#include "compiler/nvtt_blob_output.h"
#include "util.h"

using namespace Microsoft::WRL;

void compile_sound(String path)
{
	//FSBank_Init(FSBANK_FSBVERSION_FSB5, FSBANK_INIT_NORMAL, 16, ".fscache");

	
	FSBANK_SUBSOUND s;
	s.numFiles = 1;


	//FSBank_Build(&s, 1, FSBANK_FORMAT_FADPCM, FSBANK_BUILD_FSB5_DONTWRITENAMES, 0);
}

ComPtr<IDxcUtils> pUtils;
ComPtr<IDxcCompiler3> pCompiler;
ComPtr<IDxcIncludeHandler> pIncludeHandler;




IDxcBlob* compile_shader(fs::path entry, ShaderType type)
{
	QUE_PROFILE;

	std::ifstream file(entry);
	std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	auto parent_path = entry.parent_path();

	ComPtr<IDxcBlobEncoding> pSource;
	pUtils->CreateBlob(str.data(), str.size(), CP_UTF8, pSource.GetAddressOf());

	std::vector<LPCWSTR> arguments;
	// -E for the entry point (eg. 'main')
	arguments.push_back(L"-E");

	if (type == ShaderType::ST_PIXEL)
		arguments.push_back(L"ps_main");
	else if (type == ShaderType::ST_VERTEX)
		arguments.push_back(L"vs_main");

	// spirv
	arguments.push_back(L"-spirv");

	// -T for the target profile (eg. 'ps_6_6')
	arguments.push_back(L"-T");

	if (type == ShaderType::ST_PIXEL)
		arguments.push_back(L"ps_6_0");
	else if (type == ShaderType::ST_VERTEX)
		arguments.push_back(L"vs_6_0");

	arguments.push_back(L"-I");
	arguments.push_back(parent_path.c_str());

	DxcBuffer sourceBuffer;
	sourceBuffer.Ptr = pSource->GetBufferPointer();
	sourceBuffer.Size = pSource->GetBufferSize();
	sourceBuffer.Encoding = 0;

	ComPtr<IDxcResult> pCompileResult;
	auto hr = pCompiler->Compile(&sourceBuffer, arguments.data(), (UINT32)arguments.size(), pIncludeHandler.Get(), IID_PPV_ARGS(&pCompileResult));

	// print errors
	ComPtr<IDxcBlobEncoding> errors;
	pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

	if (errors != nullptr && errors->GetBufferSize() > 0)
	{
		std::string errorString((char*)errors->GetBufferPointer(), errors->GetBufferSize());
		std::cout << errorString << std::endl;
	}

	IDxcBlob* outputBlob;
	hr = pCompileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&outputBlob), nullptr);

	return outputBlob;
}

void compile_shaders(std::vector<fs::path> paths)
{
	QUE_PROFILE;

	fs::path shader_cache_path = ".cache/shaders";
	fs::create_directory(shader_cache_path);

	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.GetAddressOf()));
	pUtils->CreateDefaultIncludeHandler(pIncludeHandler.GetAddressOf());

	for (const auto& entry : paths)
	{
		QUE_PROFILE_SECTION("Shader Compile");
		QUE_PROFILE_TAG("Shader: ", entry.string().c_str());

		std::ifstream file(entry);
		std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		
		auto hash = get_file_hash(str);

		// only need to check vs since they share source md5
		if (!check_if_requires_recompilation(hash, shader_cache_path / (entry.stem().string() + ".vs_c")))
			continue;

		auto vs = compile_shader(entry, ShaderType::ST_VERTEX);
		auto ps = compile_shader(entry, ShaderType::ST_PIXEL);

		
		{
			std::ifstream file(entry);
			std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

			auto hash = get_file_hash(str);

			C_Shader shader{};
			shader.type = ShaderType::ST_VERTEX;
			shader.size = vs->GetBufferSize();
			shader.blob = vs->GetBufferPointer();

			std::strcpy(shader.header.hash, hash.c_str());

			std::ofstream out(shader_cache_path / (entry.stem().string() + ".vs_c"), std::ios::binary);
			shader.serialize(out);
			out.close();
		}


		// ps

		{
			std::ifstream file(entry);
			std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

			auto hash = get_file_hash(str);

			C_Shader shader{};
			shader.type = ShaderType::ST_PIXEL;
			shader.size = ps->GetBufferSize();
			shader.blob = ps->GetBufferPointer();

			std::strcpy(shader.header.hash, hash.c_str());

			std::ofstream out(shader_cache_path / (entry.stem().string() + ".ps_c"), std::ios::binary);
			shader.serialize(out);
			out.close();
		}

		vs->Release();
		ps->Release();
	}

	return;

	ShaderBundle bundle{};
	bundle.init();

	for (const auto& entry : paths)
	{
		auto vs = compile_shader(entry, ShaderType::ST_VERTEX);
		auto ps = compile_shader(entry, ShaderType::ST_PIXEL);

		bundle.add_shader(entry.stem().string().c_str(), ShaderType::ST_VERTEX, (char*)vs->GetBufferPointer(), vs->GetBufferSize());
		bundle.add_shader(entry.stem().string().c_str(), ShaderType::ST_PIXEL, (char*)ps->GetBufferPointer(), ps->GetBufferSize());

		vs->Release();
		ps->Release();
	}

	std::ofstream out("pc_vulkan.shbnd", std::ios::binary);
	bundle.serialize(out);
}



ref<NvttBlob> compile_texture(fs::path path, fs::path output_path)
{
	QUE_PROFILE;

	nvtt::Context context;
	context.enableCudaAcceleration(true);

	ref<NvttBlob> blob = std::make_shared<NvttBlob>();

	nvtt::Surface image;
	image.load(path.string().c_str());

	nvtt::OutputOptions outputOptions;
	outputOptions.setContainer(nvtt::Container::Container_DDS10);
	outputOptions.setOutputHandler(blob.get());

	nvtt::CompressionOptions compressionOptions;
	compressionOptions.setFormat(nvtt::Format_BC7);

	context.outputHeader(image, 1, compressionOptions, outputOptions);
	context.compress(image, 0, 0, compressionOptions, outputOptions);
	return blob;
}

void compile_texture_worker(fs::path source_path, fs::path source_asset_path, fs::path cache_path)
{
	std::ifstream asset(source_path, std::ios::binary);
	std::string str((std::istreambuf_iterator<char>(asset)), std::istreambuf_iterator<char>());


	fs::path asset_relative_path = source_asset_path.lexically_relative(source_path);
	fs::create_directories(cache_path / asset_relative_path.parent_path());

	asset_relative_path.replace_extension("tex_c");

	C_Texture ct{};

	auto hash = get_file_hash(str);

	if (!check_if_requires_recompilation(hash, cache_path / asset_relative_path))
		return;

	std::strcpy(ct.header.hash, hash.c_str());


	auto tex = compile_texture(source_asset_path, cache_path / asset_relative_path);

	ct.dds_blob = malloc(tex->size);
	ct.blob_size = tex->size;
	memcpy(ct.dds_blob, tex->data, tex->size);

	std::ofstream out(cache_path / asset_relative_path, std::ios::binary);
	ct.serialize(out);
}

void compile_textures(fs::path source_path, std::vector<fs::path> paths, fs::path output_path)
{
	QUE_PROFILE;



	fs::path cache_path = output_path;

	for (const auto& path : paths)
	{
		compile_texture_worker(source_path, path, cache_path);
	}
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

void ResourceCompiler::Compile(fs::path source_data_path, fs::path output_dir)
{
	QUE_PROFILE;

	// Get the command-line string
	LPWSTR commandLine = GetCommandLineW();

	// Parse the command line into an array of arguments
	int argc;
	LPWSTR* argv = CommandLineToArgvW(commandLine, &argc);
	if (argv == nullptr) {
		std::cerr << "CommandLineToArgvW failed\n";
	}

	if (argc < 2)
		return;

	// Store arguments in a vector of std::string
	std::vector<std::string> args;
	for (int i = 0; i < argc; ++i) {
		args.push_back(ws2s(argv[i])); // Convert wide string to std::string
	}

	if (args[1] == "sc")
	{
		std::filesystem::path shader_dir = source_data_path / "shader/hlsl";
		std::vector<fs::path> paths;
		std::vector<fs::path> texture_paths;
		std::vector<fs::path> skies_path;

		for (const auto& entry : std::filesystem::directory_iterator(shader_dir))
		{
			if (entry.is_regular_file())
			{
				auto path = entry.path();
				auto ext = path.extension();

				if (ext == ".hlsl")
				{
					paths.push_back(path);
				}
			}
		}

		for (const auto& entry : std::filesystem::recursive_directory_iterator(source_data_path))
		{
			if (entry.is_regular_file())
			{
				auto path = entry.path();
				auto ext = path.extension();

				if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
				{
					texture_paths.push_back(path);
				}

				if (ext == ".sky")
				{
					skies_path.push_back(path);
				}
			}
		}

		//compile_shaders(paths);
		compile_textures(source_data_path, texture_paths, output_dir);
		compile_skies(source_data_path, skies_path, output_dir);
	}
}
