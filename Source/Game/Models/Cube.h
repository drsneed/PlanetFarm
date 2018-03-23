#pragma once
#include <Core/StdIncludes.h>
#include <Core/GraphicsWindow.h>

class Cube
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

	Cube();
	~Cube();

	ID3D11Buffer** GetVertexBufferAddr() { return &m_vertexBuffer; }
	ID3D11Buffer* GetIndexBuffer() const { return m_indexBuffer; }
	int GetVertexCount() const { return m_vertexCount; }
};