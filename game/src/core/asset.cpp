#include "pch.h"

#include "asset.h"

#include <fstream>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


#include <common/GraphicsAPI.h>
#include <common/GraphicsAPI_Vulkan.h>
#include <lib/stb_image.h>

#if defined(__ANDROID__)
#include <assimp/port/AndroidJNI/AndroidJNIIOSystem.h>
#endif
#include <app.h>

#include <lib/json.hpp>
#include "profiler.h"

Asset* Asset::Instance;

using json = nlohmann::json;

Asset::Asset(void* android_ass)
{
	Instance = this;



#if defined(__ANDROID__)
	m_android_asset_manager = (AAssetManager*)android_ass;
#endif
}

Asset::Asset()
{
	Instance = this;

}

Asset::~Asset()
{
}


std::vector<char> Asset::read_all_bytes(String path)
{

#if defined(__ANDROID__)
    AAsset* file = AAssetManager_open(m_android_asset_manager, path.c_str(), AASSET_MODE_BUFFER);
    size_t fileLength = AAsset_getLength(file);
    std::vector<char> binary(fileLength);
    AAsset_read(file, (void*)binary.data(), fileLength);
    AAsset_close(file);
    return binary;
#else

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

Model Asset::load_model(GraphicsAPI_Vulkan& gapi, Path path)
{
	QUE_PROFILE;

	Model mod;

	Assimp::Importer imp;


#if defined(__ANDROID__)
	Assimp::AndroidJNIIOSystem* ioSystem = new Assimp::AndroidJNIIOSystem(App::androidApp->activity);
	if (nullptr != ioSystem) {
		imp.SetIOHandler(ioSystem);
	}
#endif

	// create models
	const aiScene* scene = imp.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);


	for (int m = 0; m < scene->mNumMeshes; m++)
	{
		std::vector<Vertex> vertices;
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
		internal_mesh.vertex_buffer = gapi.CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::VERTEX, sizeof(float) * 14, sizeof(Vertex) * vertices.size(), vertices.data() });
		internal_mesh.index_buffer = gapi.CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::INDEX, sizeof(uint32_t), sizeof(uint32_t) * indices.size(), indices.data() });
		internal_mesh.index_count = indices.size();

		auto model_directory = path.parent_path();

		if (mesh->mMaterialIndex >= 0)
		{
			// check if material index is in map 
			// if not, load material
			
			
			if (mod.materials.find(mesh->mMaterialIndex) == mod.materials.end())
			{
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

				Material mat;

				mat.diff = try_to_load_texture_type(gapi, scene, material, aiTextureType_DIFFUSE, model_directory.string());
				mat.norm = try_to_load_texture_type(gapi, scene, material, aiTextureType_NORMALS, model_directory.string());
				mat.orm = try_to_load_texture_type(gapi, scene, material, aiTextureType_METALNESS, model_directory.string());

				mod.materials.emplace(mesh->mMaterialIndex, mat);
			}
			
			internal_mesh.material_index = mesh->mMaterialIndex;
		}
		mod.meshes.push_back(internal_mesh);
	}

	return mod;
}


GraphicsAPI::Image Asset::load_image(GraphicsAPI_Vulkan& gapi, String path, TextureType type)
{
	QUE_PROFILE;

	stbi_set_flip_vertically_on_load(true);

	if (!std::filesystem::is_regular_file("./" + path))
	{
		std::cout << "[asset] texture doest exist: " << path << std::endl;
		return gapi.tex_placeholder;
	}

	GraphicsAPI::Image img;

	VkFormat format;

	// https://www.reddit.com/r/vulkan/comments/wksa4z/strange_issue_with_normal_maps_in_pbr_shader/
	if (type == TT_DIFFUSE)
		format = VK_FORMAT_R8G8B8A8_SRGB;
	else if (type == TT_NORMAL)
		format = VK_FORMAT_R8G8B8A8_UNORM;
	else if (type == TT_HDRI)
		format = VK_FORMAT_R32G32B32A32_SFLOAT;

	assert(format != VK_FORMAT_UNDEFINED);

	auto device = gapi.GetDevice();
	auto allocator = gapi.GetAllocator();
	int texWidth, texHeight, texChannels;

	
	void* pixel_ptr;


	{
		QUE_PROFILE_SECTION("Load data from disk");
#if defined(__ANDROID__)
			auto am = App::androidApp->activity->assetManager;
		AAsset* asset = AAssetManager_open(am, path.c_str(), AASSET_MODE_BUFFER);

		long fileLength = AAsset_getLength(asset);
		char* buffer = new char[fileLength];
		int bytesRead = AAsset_read(asset, buffer, fileLength);
		if (type == TT_HDRI)
		{
			float* temp = stbi_loadf_from_memory((stbi_uc*)buffer, fileLength, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			pixel_ptr = temp;
		}
		else
			pixel_ptr = stbi_load_from_memory((stbi_uc*)buffer, fileLength, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
#else
		if (type == TT_HDRI)
		{
			float* temp = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			pixel_ptr = temp;
		}
		else
			pixel_ptr = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
#endif
	}

	if (!pixel_ptr)
		DEBUG_BREAK;

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	// allocate more memory if it's an hdri since we're using floats
	if (type == TT_HDRI)
		imageSize *= sizeof(float);

	GraphicsAPI::Buffer stagingBuffer;

	VkBufferCreateInfo stagingBufferInfo = {};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.pNext = nullptr;
	stagingBufferInfo.size = imageSize;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	//let the VMA library know that this data should be on CPU RAM
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

	auto result = vmaCreateBuffer(allocator, &stagingBufferInfo, &vmaallocInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, nullptr);

	gapi.SetDebugName("staging buffer img", stagingBuffer.buffer);


	void* data;
	vmaMapMemory(allocator, stagingBuffer.allocation, &data);
	memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));
	vmaUnmapMemory(allocator, stagingBuffer.allocation);

	gapi.MainDeletionQueue.push_function([&]() {
		//vmaDestroyBuffer(*allocator, stagingBuffer.buffer, stagingBuffer.allocation);
	});

	stbi_image_free(pixel_ptr);

	VkExtent3D imageExtent;
	imageExtent.width = static_cast<uint32_t>(texWidth);
	imageExtent.height = static_cast<uint32_t>(texHeight);
	imageExtent.depth = 1;

	// create image
	VkImageCreateInfo info = { };
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.imageType = VK_IMAGE_TYPE_2D;
	info.format = format;
	info.extent = imageExtent;
	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VULKAN_CHECK_NOMSG(vmaCreateImage(allocator, &info, &dimg_allocinfo, &img.image, &img.allocation, nullptr));

	VkImageViewCreateInfo ivinfo = {};
	ivinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivinfo.pNext = nullptr;

	ivinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivinfo.image = img.image;
	ivinfo.format = format;
	ivinfo.subresourceRange.baseMipLevel = 0;
	ivinfo.subresourceRange.levelCount = 1;
	ivinfo.subresourceRange.baseArrayLayer = 0;
	ivinfo.subresourceRange.layerCount = 1;
	ivinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkCreateImageView(device, &ivinfo, nullptr, &img.view);

	{
		QUE_PROFILE_SECTION("Upload image to gpu");

		gapi.immediate_submit([&](VkCommandBuffer cmd) {
			VkImageSubresourceRange range{};
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkImageMemoryBarrier imageBarrier_toTransfer = {};
			imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

			imageBarrier_toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier_toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toTransfer.image = img.image;
			imageBarrier_toTransfer.subresourceRange = range;

			imageBarrier_toTransfer.srcAccessMask = 0;
			imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;


			//barrier the image into the transfer-receive layout
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = imageExtent;

			vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer, img.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			VkImageMemoryBarrier imgBarrier_toShaderReadable = imageBarrier_toTransfer;
			imgBarrier_toShaderReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imgBarrier_toShaderReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			imgBarrier_toShaderReadable.image = img.image;
			imgBarrier_toShaderReadable.subresourceRange = range;

			imgBarrier_toShaderReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imgBarrier_toShaderReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imgBarrier_toShaderReadable);
		});
	}

	gapi.SetDebugName("texture: " + path, img.image);
	gapi.SetDebugName("texture view: " + path, img.view);

	return img;
}

Model Asset::load_model_json(GraphicsAPI_Vulkan& gapi, Path path)
{
	QUE_PROFILE;

	Model model;


	auto loadMeshes = [&](String modelPath) -> std::vector<Mesh> {

		std::vector<Mesh> meshes;

		Assimp::Importer imp;
#if defined(__ANDROID__)
		Assimp::AndroidJNIIOSystem* ioSystem = new Assimp::AndroidJNIIOSystem(App::androidApp->activity);
		if (nullptr != ioSystem) {
			imp.SetIOHandler(ioSystem);
		}
#endif

		const aiScene* scene = imp.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
		for (int m = 0; m < scene->mNumMeshes; m++)
		{
			std::vector<Vertex> vertices;
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

			internal_mesh.vertex_buffer = gapi.CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::VERTEX, sizeof(float) * 14, sizeof(Vertex) * vertices.size(), vertices.data() });
			internal_mesh.index_buffer = gapi.CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::INDEX, sizeof(uint32_t), sizeof(uint32_t) * indices.size(), indices.data() });
			internal_mesh.index_count = indices.size();

			meshes.push_back(internal_mesh);
		}


		return meshes;
	};
		
	auto file_bytes = Asset::Instance->read_all_bytes(path.string());
	auto desc = json::parse(file_bytes);

	auto desc_directory = path.parent_path().string();

	model.meshes = loadMeshes(desc_directory + "/" + (String)desc["file"]);

	for (auto& mesh : model.meshes) {
		mesh.material_index = 0;
	}

	for (auto material : desc["materials"])
	{
		Material m;
		m.diff = load_image(gapi, desc_directory + "/" + (String)material["diffuse"], TT_DIFFUSE);
		m.norm = load_image(gapi, desc_directory + "/" + (String)material["normal"], TT_NORMAL);
		m.orm = load_image(gapi, desc_directory + "/" + (String)material["orm"], TT_DIFFUSE);

		model.materials.emplace((int)material["id"], m);
	}

	return model;
}

GraphicsAPI::Image Asset::try_to_load_texture_type(GraphicsAPI_Vulkan& gapi, const aiScene* scene, aiMaterial* material, aiTextureType type, String root_path)
{
	aiString path;
	material->GetTexture(type, 0, &path);

	const aiTexture* tex = scene->GetEmbeddedTexture(path.C_Str());

	//load texture with stb
	if (!tex && path.length > 0)
	{
		return load_image(gapi, root_path + "/" + std::string(path.C_Str()), type == aiTextureType_NORMALS ? TT_NORMAL : TT_DIFFUSE);
	}
	
	return gapi.tex_placeholder;
} 
