#pragma once
#include "StdIncludes.h"
#include "GraphicsWindow.h"

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

	_declspec(align(16))
	struct VoxelInstance
	{
		XMFLOAT4 Position;
		XMFLOAT3 Color;
	};
