#include "MapRenderer.h"
#include <Core/ColorConverter.h>

WorldRenderer::WorldRenderer()
	: m_gridShader(LR"(Data/Grid.fx)", Shader::Vertex | Shader::Pixel)
	, m_gridInputLayout(nullptr)
{

	auto device = GraphicsWindow::GetInstance()->GetDevice();

	D3D11_INPUT_ELEMENT_DESC layout2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (!D3DCheck(device->CreateInputLayout(layout2, _countof(layout2), m_gridShader.GetByteCode(Shader::Vertex)->GetBufferPointer(),
		m_gridShader.GetByteCode(Shader::Vertex)->GetBufferSize(), &m_gridInputLayout),
		L"ID3D11Device::CreateInputLayout (WorldRenderer, grid)")) return;

	m_gridShader.ReleaseByteCode();
}

WorldRenderer::~WorldRenderer()
{
	if (m_gridInputLayout)
		m_gridInputLayout->Release();
}

void WorldRenderer::RenderGrid(std::shared_ptr<CameraBase>& camera)
{
	auto context = GraphicsWindow::GetInstance()->GetContext();

	context->IASetInputLayout(m_gridInputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	UINT stride = sizeof(WorldGrid::Vertex);
	UINT offset = 0;

	context->VSSetShader(m_gridShader.GetVertexShader(), nullptr, 0);
	context->PSSetShader(m_gridShader.GetPixelShader(), nullptr, 0);

	auto cameraBuffer = camera->GetConstantBuffer();
	context->VSSetConstantBuffers(0, 1, &cameraBuffer);

	context->IASetVertexBuffers(0, 1, m_grid.GetVertexBufferAddr(), &stride, &offset);

	context->Draw(m_grid.GetVertexCount(), 0);
}
