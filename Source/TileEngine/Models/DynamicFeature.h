#pragma once
#include <Core/StdIncludes.h>
#include <Core/GraphicsWindow.h>

class DynamicFeature
{
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	int m_vertexCount;
public:
	struct Vertex
	{
		FLOAT x, y, z;
		FLOAT u, v;
		FLOAT nx, ny, nz;
	};

	DynamicFeature();
	~DynamicFeature();

	//void AddVertexData(const std::vector<Vertex>& vertices);
	ID3D11Buffer** GetVertexBufferAddr() { return &m_vertexBuffer; }
	ID3D11Buffer* GetIndexBuffer() const { return m_indexBuffer; }
	int GetVertexCount() const { return m_vertexCount; }
};