#pragma once
#include "StdIncludes.h"
#include "GraphicsWindow.h"

class WorldGrid
{
	ID3D11Buffer* m_vertexBuffer;
	int m_vertexCount;
public:
	struct Vertex
	{
		FLOAT x, y, z;
		FLOAT r, g, b;
	};

	WorldGrid();
	~WorldGrid();

	ID3D11Buffer** GetVertexBufferAddr() { return &m_vertexBuffer; }
	int GetVertexCount() const { return m_vertexCount; }
};