#include "DynamicFeatureView.h"

DynamicFeatureView::DynamicFeatureView(DynamicFeature* parent, const std::vector<XMFLOAT2>& vertex_data)
	: vertex_buffer(nullptr)
	, parent(parent)
	, vertex_count(0)
{
	CreateBuffers(vertex_data);
}

void DynamicFeatureView::CreateBuffers(const std::vector<XMFLOAT2>& vertex_data)
{
	ASSERT(vertex_buffer == nullptr);
	ASSERT(vertex_count == 0);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.ByteWidth = sizeof(XMFLOAT2) * vertex_data.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubresource;
	ZeroMemory(&vertexSubresource, sizeof(vertexSubresource));
	vertexSubresource.pSysMem = &vertex_data[0];

	auto device = GraphicsWindow::GetInstance()->GetDevice();

	if (!D3DCheck(device->CreateBuffer(&vertexBufferDesc, &vertexSubresource, &vertex_buffer),
		L"ID3D11Device::CreateBuffer (DynamicFeature::View, VertexBuffer)")) return;
	vertex_count = vertex_data.size();
}