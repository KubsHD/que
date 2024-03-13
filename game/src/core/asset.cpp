#include "asset.h"

#include <fstream>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


#include <common/GraphicsAPI.h>
#include <common/GraphicsAPI_Vulkan.h>

Asset::Asset(void* android_ass)
{
#if defined(__ANDROID__)
	m_android_asset_manager = (AAssetManager*)android_ass;
#endif
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

Mesh Asset::load_mesh(GraphicsAPI_Vulkan& gapi, String path)
{
	Mesh m;

	Assimp::Importer imp;


	// create models
	const aiScene* scene = imp.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (int m = 0; m < scene->mNumMeshes; m++)
	{
		auto mesh = scene->mMeshes[m];
		
		for (size_t i = 0; i < mesh->mNumVertices; i++) {
			vertices.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z,
				mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
		}

		for (size_t i = 0; i < mesh->mNumFaces; i++) {
			indices.push_back(mesh->mFaces[i].mIndices[0]);
			indices.push_back(mesh->mFaces[i].mIndices[1]);
			indices.push_back(mesh->mFaces[i].mIndices[2]);
		}
	}

	m.vertex_buffer = (VkBuffer*)gapi.CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::VERTEX, sizeof(float) * 8, sizeof(Vertex) * vertices.size(), vertices.data() });
	m.index_buffer = (VkBuffer*)gapi.CreateBuffer({ GraphicsAPI::BufferCreateInfo::Type::INDEX, sizeof(uint32_t), sizeof(uint32_t) * indices.size(), indices.data() });
	m.index_count = indices.size();
	// add debug names
	//gapi.SetDebugName("vertex_buffer", (void*)m.vertex_buffer);

	return m;
}
