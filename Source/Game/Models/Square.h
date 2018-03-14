#pragma once
#include <Core/StdIncludes.h>
#include <Core/GraphicsWindow.h>

class Square
{
	ID3D11Buffer* m_vertexBuffer;
	int m_vertexCount;
public:
	struct Vertex
	{
		FLOAT x, y;
	};

	Square();
	~Square();

	ID3D11Buffer** GetVertexBufferAddr() { return &m_vertexBuffer; }
	int GetVertexCount() const { return m_vertexCount; }
};
