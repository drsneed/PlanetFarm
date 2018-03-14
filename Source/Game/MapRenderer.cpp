#include "MapRenderer.h"
#include <Core/ColorConverter.h>

MapRenderer::MapRenderer()
	: m_gridShader(LR"(Data/Grid.fx)", Shader::Vertex | Shader::Pixel)
	, m_gridInputLayout(nullptr)
	, m_squareShader(LR"(Data/Square.fx)", Shader::Vertex | Shader::Pixel)
	, m_squareInputLayout(nullptr)
	, m_perObjectBuffer(nullptr)
{

	auto device = GraphicsWindow::GetInstance()->GetDevice();

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (!D3DCheck(device->CreateInputLayout(layout, _countof(layout), m_gridShader.GetByteCode(Shader::Vertex)->GetBufferPointer(),
		m_gridShader.GetByteCode(Shader::Vertex)->GetBufferSize(), &m_gridInputLayout),
		L"ID3D11Device::CreateInputLayout (MapRenderer, grid)")) return;

	m_gridShader.ReleaseByteCode();

	D3D11_INPUT_ELEMENT_DESC layout2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (!D3DCheck(device->CreateInputLayout(layout2, _countof(layout2), m_squareShader.GetByteCode(Shader::Vertex)->GetBufferPointer(),
		m_squareShader.GetByteCode(Shader::Vertex)->GetBufferSize(), &m_squareInputLayout),
		L"ID3D11Device::CreateInputLayout (MapRenderer, square)")) return;

	m_squareShader.ReleaseByteCode();


	D3D11_BUFFER_DESC desc;

	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(SquarePerObjectBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (!D3DCheck(device->CreateBuffer(&desc, nullptr, &m_perObjectBuffer),
		L"ID3D11Device::CreateBuffer (MapRenderer, SquarePerObjectBuffer)")) return;
}

MapRenderer::~MapRenderer()
{
	if (m_gridInputLayout)
		m_gridInputLayout->Release();
	if (m_squareInputLayout)
		m_squareInputLayout->Release();
	if (m_perObjectBuffer)
		m_perObjectBuffer->Release();
}

void MapRenderer::DrawGrid(std::shared_ptr<Camera>& camera)
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

void MapRenderer::_UploadPerObjectBuffer(ID3D11DeviceContext* context, const SquarePerObjectBuffer& perObject)
{
	// Update per resize buffer
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	if (!D3DCheck(context->Map(m_perObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes),
		L"ID3D11DeviceContext::Map (MapRenderer::UploadPerObjectBuffer)")) return;
	memcpy_s(mappedRes.pData, mappedRes.RowPitch, &perObject, sizeof(perObject));
	context->Unmap(m_perObjectBuffer, 0);
}

void MapRenderer::DrawSquare(std::shared_ptr<Camera>& camera, XMFLOAT2 position, float width, unsigned color)
{
	auto context = GraphicsWindow::GetInstance()->GetContext();
	__declspec(align(16)) SquarePerObjectBuffer object{};
	object.ObjectColor = ConvertColor(color);
	object.ObjectPosition = position;
	object.ObjectSize = XMFLOAT2(width, width);
	_UploadPerObjectBuffer(context, object);
	context->IASetInputLayout(m_squareInputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	UINT stride = sizeof(Square::Vertex);
	UINT offset = 0;

	context->VSSetShader(m_squareShader.GetVertexShader(), nullptr, 0);
	context->PSSetShader(m_squareShader.GetPixelShader(), nullptr, 0);

	auto cameraBuffer = camera->GetConstantBuffer();
	context->VSSetConstantBuffers(0, 1, &cameraBuffer);
	//context->VSSetConstantBuffers(1, 1, &m_perObjectBuffer);
	context->IASetVertexBuffers(0, 1, m_square.GetVertexBufferAddr(), &stride, &offset);

	auto vertex_count = m_square.GetVertexCount();
	context->Draw(vertex_count, 0);
}
