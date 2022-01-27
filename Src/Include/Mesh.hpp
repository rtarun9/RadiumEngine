#include "Pch.hpp"

#include "Graphics/Buffer.hpp"

struct Vertex
{
	dx::XMFLOAT3 position;
	dx::XMFLOAT3 normal;
	dx::XMFLOAT2 texCoords;
};

struct Transform
{
	dx::XMVECTOR translation;
	dx::XMVECTOR scale;
	dx::XMVECTOR rotation;
};
class Mesh
{
public:
	void Init(const wrl::ComPtr<ID3D11Device>& device, const char* filePath);
	void Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

public:
	VertexBuffer<Vertex> m_VertexBuffer;
	Transform m_Transform;
};