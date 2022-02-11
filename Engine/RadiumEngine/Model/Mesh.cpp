#include "Model/Mesh.hpp"

namespace rad
{
	void Mesh::Init(ID3D11Device* device, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices, std::vector<Texture>&& textures)
	{
		m_IndicesCount = indices.size();

		m_Vertices = std::move(vertices);
		m_Indices = std::move(indices);
		m_Textures = std::move(textures);


		m_VertexBuffer.Init(device, std::move(m_Vertices));
		m_IndexBuffer.Init(device, std::move(m_Indices));

		vertices.clear();
		indices.clear();
		textures.clear();
	}

	void Mesh::Draw(ID3D11DeviceContext* deviceContext)
	{
		m_VertexBuffer.Bind(deviceContext);
		m_IndexBuffer.Bind(deviceContext);

		// Not using texture class's binding function here for now since individually binding them is causing some issues (wrong shader resources are getting bound) : Might change soon.
		ID3D11ShaderResourceView *shaderResourceViews[] =
		{
			m_Textures[TextureTypes::TextureDiffuse].m_TextureView.Get(),
			m_Textures[TextureTypes::TextureSpecular].m_TextureView.Get(),
			m_Textures[TextureTypes::TextureNormal].m_TextureView.Get(),
			m_Textures[TextureTypes::TextureHeight].m_TextureView.Get()
		};

		deviceContext->PSSetShaderResources(0, _countof(shaderResourceViews), shaderResourceViews);

		deviceContext->DrawIndexed(m_IndexBuffer.m_Size, 0, 0);
	}
}
