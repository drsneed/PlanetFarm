#include "Grid.h"

#define GRID_LENGTH 20

WorldGrid::WorldGrid()
	: m_vertexBuffer(nullptr)
{
	std::vector<Vertex> verts;
	for (int x = -GRID_LENGTH; x < GRID_LENGTH +1; ++x)
	{
		Vertex s, t;
		s.x = x;
		s.y = 0;
		s.z = -GRID_LENGTH;


		t.x = x;
		t.y = 0;
		t.z = GRID_LENGTH;


		if(x == 0)
		{
			s.r = 0.0f;
			s.g = 1.0f;
			s.b = 0.0f;
			t.r = 0.0f;
			t.g = 1.0f;
			t.b = 0.0f;
		}
		else
		{
			s.r = 1.0f;
			s.g = 0.5f;
			s.b = 0.0f;
			t.r = 1.0f;
			t.g = 0.5f;
			t.b = 0.0f;
			
		}

		verts.push_back(s);
		verts.push_back(t);
	}

	for (int z = -GRID_LENGTH; z < GRID_LENGTH + 1; ++z)
	{
		Vertex s, t;
		s.x = -GRID_LENGTH;
		s.y = 0;
		s.z = z;

		t.x = GRID_LENGTH;
		t.y = 0;
		t.z = z;

		if (z == 0)
		{
			s.r = 0.0f;
			s.g = 1.0f;
			s.b = 0.0f;
			t.r = 0.0f;
			t.g = 1.0f;
			t.b = 0.0f;
		}
		else
		{
			s.r = 1.0f;
			s.g = 0.5f;
			s.b = 0.0f;
			t.r = 1.0f;
			t.g = 0.5f;
			t.b = 0.0f;

		}
		verts.push_back(s);
		verts.push_back(t);
	}


	Vertex v;
	v.y = 0;
	v.x = -0.5f;
	v.z = -0.5f;
	v.r = 0.0f;
	v.g = 1.0f;
	v.b = 0.0f;
	verts.push_back(v);
	v.y = 0;
	v.x = 0.5f;
	v.z = 0.5f;
	v.r = 0.0f;
	v.g = 1.0f;
	v.b = 0.0f;
	verts.push_back(v);

	v.y = 0;
	v.x = 0.5f;
	v.z = -0.5f;
	v.r = 0.0f;
	v.g = 1.0f;
	v.b = 0.0f;
	verts.push_back(v);
	v.y = 0;
	v.x = -0.5f;
	v.z = 0.5f;
	v.r = 0.0f;
	v.g = 1.0f;
	v.b = 0.0f;
	verts.push_back(v);

	m_vertexCount = verts.size();

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubresource;
	ZeroMemory(&vertexSubresource, sizeof(vertexSubresource));
	vertexSubresource.pSysMem = &verts[0];

	auto device = GraphicsWindow::GetInstance()->GetDevice();

	if (!D3DCheck(device->CreateBuffer(&vertexBufferDesc, &vertexSubresource, &m_vertexBuffer),
		L"ID3D11Device::CreateBuffer (Cube, VertexBuffer)")) return;
}

WorldGrid::~WorldGrid()
{
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
	}
}