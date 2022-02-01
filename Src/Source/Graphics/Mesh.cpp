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

		// Not using texture class's binding function here for now since individually binding them is causing some issues (wrong shader resources are getting bound) : Might change soon.
		ID3D11ShaderResourceView *shaderResourceViews[] =
		{
			m_Textures[0].m_TextureView.Get(),
			m_Textures[1].m_TextureView.Get(),
			m_Textures[2].m_TextureView.Get(),
			m_Textures[3].m_TextureView.Get()
		};

		deviceContext->PSSetShaderResources(0, _countof(shaderResourceViews), shaderResourceViews);

		deviceContext->DrawIndexed(m_Indices.size(), 0, 0);
	}
}
