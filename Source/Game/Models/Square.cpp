#include "Square.h"

#define GRID_LENGTH 20

Square::Square()
	: m_vertexBuffer(nullptr)
	, m_vertexCount(5)
{

	Vertex verts[5];
	verts[0].x = -0.5f;
	verts[0].y = -0.5f;

	verts[1].x = 0.5f;
	verts[1].y = -0.5f;

	verts[2].x = 0.5f;
	verts[2].y = 0.5f;

	verts[3].x = -0.5f;
	verts[3].y = 0.5f;

	verts[4].x = -0.5f;
	verts[4].y = -0.5f;
	/*verts[0].x = 0.f;
	verts[0].y = 0.f;

	verts[1].x = 1.f;
	verts[1].y = 0.f;

	verts[2].x = 1.0f;
	verts[2].y = 1.0f;

	verts[3].x = 0.f;
	verts[3].y = 1.f;

	verts[4].x = 0.f;
	verts[4].y = 0.f;*/


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
		L"ID3D11Device::CreateBuffer (Square, VertexBuffer)")) return;
}

Square::~Square()
{
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
	}
}