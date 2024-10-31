#include "pch.h"

#include "asset_manager.h"
#include <common/DebugOutput.h>
#include <core/profiler.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <gfx/vertex.h>

fs::path root_path;

std::string cache_path;

std::unordered_map<std::string, std::shared_ptr<Sound>> AssetManager::m_sound_cache;

AudioSystem* AssetManager::m_audio_system_reference;

void AssetManager::Init(AudioSystem& asys)
{
	m_audio_system_reference = &asys;

	cache_path = ".cache/";

#if _DEBUG

	if (!fs::exists(cache_path))
		fs::create_directory(cache_path);

	root_path = fs::current_path();

	auto test_path = root_path;
	test_path.append(".gitignore");

	// iterate up until we are in directory with .gitignore file
	while (!fs::exists(test_path))
	{
		root_path = root_path.parent_path();
		
		test_path = root_path;
		test_path.append(".gitignore");
	}

	root_path.append("game\\data");

	LOG_INFO("Root path: " << root_path.string().c_str());

#else
	root_path = "data/";
#endif
}

std::vector<char> AssetManager::read_all_bytes(String path)
{
	fs::path real_path = root_path / path;
	return read_all_bytes_raw(real_path.string());
}

std::vector<char> AssetManager::read_all_bytes_raw(String path)
{
#if defined(__ANDROID__)
	AAsset* file = AAssetManager_open(m_android_asset_manager, path.c_str(), AASSET_MODE_BUFFER);
	size_t fileLength = AAsset_getLength(file);
	std::vector<char> binary(fileLength);
	AAsset_read(file, (void*)binary.data(), fileLength);
	AAsset_close(file);
	return binary;
#else
	if (!fs::exists(path))
		abort();

	std::ifstream stream(path, std::fstream::in | std::fstream::binary | std::fstream::ate);
	if (!stream.is_open()) {
		std::cout << "Could not read file " << path.c_str() << ". File does not exist." << std::endl;
		return {};
	}
	std::streamoff size = stream.tellg();
	std::vector<char> output(static_cast<size_t>(size));
	stream.seekg(0, std::fstream::beg);
	stream.read(output.data(), size);
	stream.close();
	return output;
#endif
}

GPUImage AssetManager::load_image(String path, TextureType type)
{
	QUE_PROFILE;
	QUE_PROFILE_TAG("Image path", path.c_str());
	
	auto bytes = AssetManager::read_all_bytes(path);

	stbi_set_flip_vertically_on_load(true);

	VkFormat format;

	// https://www.reddit.com/r/vulkan/comments/wksa4z/strange_issue_with_normal_maps_in_pbr_shader/
	if (type == TT_DIFFUSE)
		format = VK_FORMAT_R8G8B8A8_SRGB;
	else if (type == TT_NORMAL)
		format = VK_FORMAT_R8G8B8A8_UNORM;
	else if (type == TT_HDRI)
		format = VK_FORMAT_R32G32B32A32_SFLOAT;

	assert(format != VK_FORMAT_UNDEFINED);

	int texWidth, texHeight, texChannels;

	void* pixel_ptr;

	if (type == TT_HDRI)
	{
		float* temp = stbi_loadf_from_memory((stbi_uc*)bytes.data(), bytes.size(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		pixel_ptr = temp;
	}
	else
		pixel_ptr = stbi_load_from_memory((stbi_uc*)bytes.data(), bytes.size(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	GPUImage img = GfxDevice::create_image(pixel_ptr, VkExtent2D{ (uint32_t)texWidth, (uint32_t)texHeight }, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, false);

	return img;
}

std::shared_ptr<Sound> AssetManager::load_sound(String path)
{
	QUE_PROFILE;
	QUE_PROFILE_TAG("Sound path", path.c_str());

	const fs::path real_path = root_path / path;

	Sound* snd;

	for (auto [k, v] : m_sound_cache)
	{
		if (k == path)
			return v;
	}

	snd = m_audio_system_reference->create_sound(real_path.string());
	snd->path = path;

	m_sound_cache.emplace(path, std::shared_ptr<Sound>(snd));

	return m_sound_cache[path];
}

Model AssetManager::load_model(Path path)
{
	QUE_PROFILE;

	const fs::path real_path = root_path / path;

	Model mod;

	mod.name = path.string();

	Assimp::Importer imp;

#if defined(__ANDROID__)
	Assimp::AndroidJNIIOSystem* ioSystem = new Assimp::AndroidJNIIOSystem(App::androidApp->activity);
	if (nullptr != ioSystem) {
		imp.SetIOHandler(ioSystem);
	}
#endif

	// create models
	const aiScene* scene = imp.ReadFile(real_path.string(), aiProcess_Triangulate | aiProcess_CalcTangentSpace);


	for (int m = 0; m < scene->mNumMeshes; m++)
	{
		std::vector<Vertex2> vertices;
		std::vector<uint32_t> indices;

		Mesh internal_mesh;


		auto mesh = scene->mMeshes[m];

		for (size_t i = 0; i < mesh->mNumVertices; i++) {
			vertices.push_back({
				mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z,
				mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z,
				mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z,
				mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z,
				mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y
				});
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		

		internal_mesh.vertex_buffer = GfxDevice::create_buffer(sizeof(Vertex2) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		internal_mesh.index_buffer = GfxDevice::create_buffer(sizeof(uint32_t) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		internal_mesh.index_count = indices.size();

		GfxDevice::upload_buffer(internal_mesh.vertex_buffer, 0, vertices.data(), sizeof(Vertex2) * vertices.size());
		GfxDevice::upload_buffer(internal_mesh.index_buffer, 0, indices.data(), sizeof(uint32_t) * indices.size());

		internal_mesh.vertices = vertices;
		internal_mesh.indices = indices;

		auto model_directory = real_path.parent_path();

		if (mesh->mMaterialIndex >= 0)
		{
			// check if material index is in map 
			// if not, load material


			if (mod.materials.find(mesh->mMaterialIndex) == mod.materials.end())
			{
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

				Material mat;

				mod.materials.emplace(mesh->mMaterialIndex, mat);
			}

			internal_mesh.material_index = mesh->mMaterialIndex;
		}
		mod.meshes.push_back(internal_mesh);
	}

	mod.sorted_meshes[PipelineType::LIT].reserve(1);
	mod.sorted_meshes[PipelineType::UNLIT].reserve(1);


	for (const auto& mesh : mod.meshes)
	{

		const auto& material = mod.materials.at(mesh.material_index);

		if (material.type == PipelineType::UNLIT)
		{
			mod.sorted_meshes[PipelineType::UNLIT].push_back(mesh);
		}
		else if (material.type == PipelineType::LIT)
		{
			mod.sorted_meshes[PipelineType::LIT].push_back(mesh);
		}
	}


	return mod;
}

Model AssetManager::load_model_json(Path path)
{
	QUE_PROFILE;

	const fs::path real_path = root_path / path;

	Model model{};

	auto loadMeshes = [&](String modelPath) -> std::vector<Mesh> {
		std::vector<Mesh> meshes;

		Assimp::Importer imp;
#if defined(__ANDROID__)
		Assimp::AndroidJNIIOSystem* ioSystem = new Assimp::AndroidJNIIOSystem(App::androidApp->activity);
		if (nullptr != ioSystem) {
			imp.SetIOHandler(ioSystem);
		}
#endif

		const aiScene* scene = imp.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
		for (int m = 0; m < scene->mNumMeshes; m++)
		{
			std::vector<Vertex2> vertices;
			std::vector<uint32_t> indices;

			Mesh internal_mesh;


			auto mesh = scene->mMeshes[m];

			for (size_t i = 0; i < mesh->mNumVertices; i++) {
				vertices.push_back({
				mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z,
				mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z,
				mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z,
				mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z,
				mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y
					});
			}

			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}

			internal_mesh.vertex_buffer = GfxDevice::create_buffer(sizeof(Vertex2) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			internal_mesh.index_buffer = GfxDevice::create_buffer(sizeof(uint32_t) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			internal_mesh.index_count = indices.size();

			GfxDevice::upload_buffer(internal_mesh.vertex_buffer, 0, vertices.data(), sizeof(Vertex2) * vertices.size());
			GfxDevice::upload_buffer(internal_mesh.index_buffer, 0, indices.data(), sizeof(uint32_t) * indices.size());

			internal_mesh.vertices = vertices;
			internal_mesh.indices = indices;

			internal_mesh.material_index = mesh->mMaterialIndex;

			meshes.push_back(internal_mesh);
		}

		return meshes;
		};

	auto file_bytes = read_all_bytes(real_path.string());
	auto desc = nlohmann::json::parse(file_bytes);

	auto desc_directory = real_path.parent_path().string();

	model.meshes = loadMeshes(desc_directory + "/" + (String)desc["file"]);

	return model;
}

nlohmann::json AssetManager::read_json(String path)
{
	auto bytes = AssetManager::read_all_bytes(path);
	return nlohmann::json::parse(bytes.begin(), bytes.end());
}