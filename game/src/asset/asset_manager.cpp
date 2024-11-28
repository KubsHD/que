#include "pch.h"

#include "asset_manager.h"

#include <common/DebugOutput.h>
#include <core/profiler.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <gfx/vertex.h>
#include <gfx/renderer2.h>

#include <asset/file/texture.h>

#define DDSKTX_IMPLEMENT
#include <lib/dds-ktx.h>

fs::path root_path;

std::string cache_path;

std::unordered_map<std::string, std::shared_ptr<Sound>> AssetManager::m_sound_cache;

std::unordered_map<std::string, std::shared_ptr<GPUImage>> m_image_cache;
std::unordered_map<std::string, std::shared_ptr<Model>> m_model_cache;



AudioSystem* AssetManager::m_audio_system_reference;
Renderer2* AssetManager::m_renderer_reference;

template<typename T>
std::shared_ptr<T> try_cache_load(String path, std::unordered_map<std::string, std::shared_ptr<T>>& cache)
{
	QUE_PROFILE;
	QUE_PROFILE_TAG("Asset path", path.c_str());
	const fs::path real_path = root_path / path;
	for (auto [k, v] : cache)
	{
		if (k == path)
			return v;
	}

	return nullptr;
}



void AssetManager::PreInit()
{
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

void AssetManager::Init(AudioSystem& asys, Renderer2& ren)
{
	m_audio_system_reference = &asys;
	m_renderer_reference = &ren;
}

void AssetManager::Destroy()
{
	for (auto [k, v] : m_sound_cache)
	{
	}

	m_sound_cache.clear();

	for (auto [k, v] : m_image_cache)
	{
		GfxDevice::destroy_image(*v);
	}

	m_image_cache.clear();

	for (auto [k, v] : m_model_cache)
	{
		for (auto& mesh : v->meshes)
		{
			GfxDevice::destroy_buffer(mesh.vertex_buffer);
			GfxDevice::destroy_buffer(mesh.index_buffer);
		}
	}

	m_model_cache.clear();

}

std::vector<char> AssetManager::read_all_bytes(String path)
{
#if _DEBUG
	if (fs::exists(cache_path / fs::path(path)))
		return read_all_bytes_raw((cache_path / fs::path(path)).string());
#endif

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


GPUImage AssetManager::try_to_load_texture_type(const aiScene* scene, aiMaterial* material, aiTextureType type, String root_path)
{
	aiString path;
	material->GetTexture(type, 0, &path);

	const aiTexture* tex = scene->GetEmbeddedTexture(path.C_Str());

	//load texture with stb
	if (!tex && path.length > 0)
	{
		return load_texture_c(root_path + "/" + std::string(path.C_Str()), type == aiTextureType_NORMALS ? TT_NORMAL : TT_DIFFUSE);
	}
	// TODO: embedded texture support

	return m_renderer_reference->texture_checker;
}

GPUImage AssetManager::load_texture_c(String path, TextureType type)
{
	QUE_PROFILE;

	if (auto cached = try_cache_load(path, m_image_cache))
	{
		return *cached;
	}

	auto new_path = fs::path(path).replace_extension(".tex_c");

#if _DEBUG
	if (!fs::exists(cache_path / new_path))
		return m_renderer_reference->texture_checker;
#else
	if (!fs::exists(root_path / new_path))
		return m_renderer_reference->texture_black;
#endif

	auto bytes = AssetManager::read_all_bytes(new_path.string());

	C_Texture tex;
	tex.read(bytes);

	VkFormat format;

	// https://www.reddit.com/r/vulkan/comments/wksa4z/strange_issue_with_normal_maps_in_pbr_shader/
	if (type == TT_DIFFUSE)
		format = VK_FORMAT_BC7_SRGB_BLOCK;
	else if (type == TT_NORMAL)
		format = VK_FORMAT_BC5_UNORM_BLOCK;
	
	assert(format != VK_FORMAT_UNDEFINED);

	ddsktx_texture_info info;

	if (!ddsktx_parse(&info, tex.dds_blob, tex.blob_size))
		abort();

	GPUImage img = GfxDevice::create_image(VkExtent2D{ (uint32_t)info.width, (uint32_t)info.height }, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true);

	for (int i = 0; i < info.num_mips; i++)
	{
		ddsktx_sub_data sub_data;
		ddsktx_get_sub(&info, &sub_data, tex.dds_blob, tex.blob_size, 0, 0, i);

		GfxDevice::upload_image(img, (void*)sub_data.buff, sub_data.size_bytes, false, 0, i);
	}

	m_image_cache.emplace(path, std::make_shared<GPUImage>(img));

	return img;
}

GPUImage AssetManager::load_texture(String path, TextureType type)
{
	QUE_PROFILE;
	QUE_PROFILE_TAG("Image path", path.c_str());
	
	if (auto cached = try_cache_load(path, m_image_cache))
	{
		return *cached;
	}

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

	GPUImage img = GfxDevice::create_image(pixel_ptr, VkExtent2D{ (uint32_t)texWidth, (uint32_t)texHeight }, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true);

	m_image_cache.emplace(path, std::make_shared<GPUImage>(img));

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

	if (auto cached = try_cache_load(path.string(), m_model_cache))
	{
		return *cached;
	}

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

		auto model_directory = path.parent_path();

		if (mesh->mMaterialIndex >= 0)
		{
			if (mod.materials.find(mesh->mMaterialIndex) == mod.materials.end())
			{
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

				MAT_Unlit::Resoruces res;

				res.diffuse = try_to_load_texture_type(scene, material, aiTextureType_DIFFUSE, model_directory.string());
				res.diffuse_sampler = m_renderer_reference->default_sampler_linear;

				MaterialInstance mat = m_renderer_reference->mat_unlit.write(GfxDevice::device, res, &m_renderer_reference->global_descriptor_allocator);

				mod.materials2.emplace(mesh->mMaterialIndex, mat);
			}

			internal_mesh.material_index = mesh->mMaterialIndex;
		}
		mod.meshes.push_back(internal_mesh);
	}

	m_model_cache.emplace(path.string(), std::make_shared<Model>(mod));

	return mod;
}

Model AssetManager::load_model_json(Path path)
{
	QUE_PROFILE;

	if (auto cached = try_cache_load(path.string(), m_model_cache))
	{
		return *cached;
	}

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
		auto real_path = (root_path / modelPath);

		const aiScene* scene = imp.ReadFile(real_path.string(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
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

	auto file_bytes = read_all_bytes(path.string());
	auto desc = nlohmann::json::parse(file_bytes);

	auto desc_directory = path.parent_path().string();

	model.meshes = loadMeshes(desc_directory + "/" + (String)desc["file"]);

	for (auto material : desc["materials"])
	{
		if (material.contains("shader"))
		{
			auto& shader_type = material["shader"];
			if (shader_type == "lit")
			{
				MAT_Lit::Resoruces rs;
				rs.diffuse = load_texture_c(desc_directory + "/" + (String)material["diffuse"], TT_DIFFUSE);
				rs.normal = load_texture_c(desc_directory + "/" + (String)material["normal"], TT_NORMAL);
				rs.orm = load_texture_c(desc_directory + "/" + (String)material["orm"], TT_DIFFUSE);
				rs.emission = m_renderer_reference->texture_black;

				rs.sampler = m_renderer_reference->default_sampler_linear;

				auto inst = m_renderer_reference->mat_lit.write(GfxDevice::device, rs, &m_renderer_reference->global_descriptor_allocator);

				model.materials2.emplace((int)material["id"], inst);
			}
			else if (shader_type == "unlit")
			{

				MAT_Unlit::Resoruces rs;
				rs.diffuse = m_renderer_reference->texture_black;
				rs.diffuse_sampler = m_renderer_reference->default_sampler_linear;

				auto inst = m_renderer_reference->mat_unlit.write(GfxDevice::device, rs, &m_renderer_reference->global_descriptor_allocator);

				model.materials2.emplace((int)material["id"], inst);
			}
		}
		else
		{
			//m.type = PipelineType::LIT;

			//// legacy
			//m.diff = load_te(desc_directory + "/" + (String)material["diffuse"], TT_DIFFUSE);
			//m.norm = load_image(desc_directory + "/" + (String)material["normal"], TT_NORMAL);
			//m.orm = load_image(desc_directory + "/" + (String)material["orm"], TT_DIFFUSE);
			//m.emission = m_api->tex_placeholder;

			MAT_Lit::Resoruces rs;
			rs.diffuse = load_texture_c(desc_directory + "/" + (String)material["diffuse"], TT_DIFFUSE);
			rs.normal = load_texture_c(desc_directory + "/" + (String)material["normal"], TT_NORMAL);
			rs.orm = load_texture_c(desc_directory + "/" + (String)material["orm"], TT_DIFFUSE);
			rs.emission = m_renderer_reference->texture_black;

			rs.sampler = m_renderer_reference->default_sampler_linear;

			auto inst = m_renderer_reference->mat_lit.write(GfxDevice::device, rs, &m_renderer_reference->global_descriptor_allocator);

			model.materials2.emplace((int)material["id"], inst);
		}

	}







	m_model_cache.emplace(path.string(), std::make_shared<Model>(model));

	return model;
}

nlohmann::json AssetManager::read_json(String path)
{
	auto bytes = AssetManager::read_all_bytes(path);
	return nlohmann::json::parse(bytes.begin(), bytes.end());
}

fs::path AssetManager::get_asset_dir()
{
	return root_path;
}
