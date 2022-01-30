#include "Graphics/Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace rad
{
	void _Mesh::Init(const wrl::ComPtr<ID3D11Device>& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		m_Vertices = vertices;
		m_Indices = indices;

		m_VertexBuffer.Init(device, m_Vertices);
		m_IndexBuffer.Init(device, m_Indices);
	}

	void _Mesh::Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		m_VertexBuffer.Bind(deviceContext);
		m_IndexBuffer.Bind(deviceContext);

		deviceContext->DrawIndexed(m_Indices.size(), 0, 0);
	}

	void Model::Init(const wrl::ComPtr<ID3D11Device>& device, const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			ErrorMessage(L"Cannot load model with path : " + StringToWString(path));
		}

		ProcessNode(device, scene->mRootNode, scene);
	}

	void Model::Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i].Draw(deviceContext);
		}
	}

	void Model::ProcessNode(const wrl::ComPtr<ID3D11Device>& device, aiNode* node, const aiScene* scene)
	{
		// process all meshes of node
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(device, mesh, scene);
		}

		// process all children of current node
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(device, node->mChildren[i], scene);
		}
	}

	void Model::ProcessMesh(const wrl::ComPtr<ID3D11Device>& device, aiMesh* mesh, const aiScene* scene)
	{
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex = {};

            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;

            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;

            // NOTE : ASSIMP CAN HAVE 8 TEX COORDS PER VERTEX, BUT USING ONLY FIRST FOR NOW
            if (mesh->mTextureCoords[0])
            {
                vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
                vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
            }
            else
            {
                vertex.texCoords.x = 0;
				vertex.texCoords.y = 0;
            }

            vertices.push_back(vertex);
        }

        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

		_Mesh newMesh;
		newMesh.Init(device, vertices, indices);
		m_Meshes.push_back(newMesh);
	}
}
