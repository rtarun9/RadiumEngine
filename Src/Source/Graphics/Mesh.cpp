#include "Graphics/Mesh.hpp"

namespace rad
{
	void Mesh::Init(const wrl::ComPtr<ID3D11Device>& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<Texture>& textures)
	{
		m_Vertices = vertices;
		m_Indices = indices;
		m_Textures = textures;

		m_VertexBuffer.Init(device, m_Vertices);
		m_IndexBuffer.Init(device, m_Indices);
	}

	void Mesh::Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		m_VertexBuffer.Bind(deviceContext);
		m_IndexBuffer.Bind(deviceContext);

		m_Textures[0].Bind(deviceContext, TextureTypes::TextureDiffuse);
		m_Textures[1].Bind(deviceContext, TextureTypes::TextureSpecular);
		m_Textures[1].Bind(deviceContext, TextureTypes::TextureNormal);
		m_Textures[1].Bind(deviceContext, TextureTypes::TextureHeight);

		deviceContext->DrawIndexed(m_Indices.size(), 0, 0);
	}
}
