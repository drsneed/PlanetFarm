#include "MapRenderer.h"
#include <Core/ColorConverter.h>
#include <TileEngine/Tile.h>
#include <cmath>

#define MAP_BOUNDS_VERTEX_COUNT 5

MapRenderer::MapRenderer(std::shared_ptr<Camera> camera)
	: m_gridShader(LR"(Data/Grid.fx)", Shader::Vertex | Shader::Pixel)
	, m_gridInputLayout(nullptr)
	, m_squareShader(LR"(Data/Square.fx)", Shader::Vertex | Shader::Pixel)
	, m_squareInputLayout(nullptr)
	, _map_bounds_shader(LR"(Data/MapBounds.fx)", Shader::Vertex | Shader::Pixel)
	, m_perObjectBuffer(nullptr)
	, _map_bounds_buffer(nullptr)
	, _map_bounds_input_layout(nullptr)
	, _cam(camera)
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

	if (!D3DCheck(device->CreateInputLayout(layout2, _countof(layout2), _map_bounds_shader.GetByteCode(Shader::Vertex)->GetBufferPointer(),
		_map_bounds_shader.GetByteCode(Shader::Vertex)->GetBufferSize(), &_map_bounds_input_layout),
		L"ID3D11Device::CreateInputLayout (MapRenderer, _map_bounds)")) return;

	_map_bounds_shader.ReleaseByteCode();

	D3D11_BUFFER_DESC desc;

	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(SquarePerObjectBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (!D3DCheck(device->CreateBuffer(&desc, nullptr, &m_perObjectBuffer),
		L"ID3D11Device::CreateBuffer (MapRenderer, SquarePerObjectBuffer)")) return;

	float w = (pow(2, TILE_MAX_ZOOM) * TILE_PIXEL_WIDTH);
	Square::Vertex verts[MAP_BOUNDS_VERTEX_COUNT];
	verts[0].x = 0.f;
	verts[0].y = 0.f;

	verts[1].x = w;
	verts[1].y = 0.f;

	verts[2].x = w;
	verts[2].y = w;

	verts[3].x = 0.f;
	verts[3].y = w;

	verts[4] = verts[0];

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Square::Vertex) * MAP_BOUNDS_VERTEX_COUNT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubresource;
	ZeroMemory(&vertexSubresource, sizeof(vertexSubresource));
	vertexSubresource.pSysMem = &verts[0];

	if (!D3DCheck(device->CreateBuffer(&vertexBufferDesc, &vertexSubresource, &_map_bounds_buffer),
		L"ID3D11Device::CreateBuffer (_map_bounds_buffer, VertexBuffer)")) return;
}

MapRenderer::~MapRenderer()
{
	if (m_gridInputLayout)
		m_gridInputLayout->Release();
	if (m_squareInputLayout)
		m_squareInputLayout->Release();
	if (m_perObjectBuffer)
		m_perObjectBuffer->Release();
	if (_map_bounds_buffer)
		_map_bounds_buffer->Release();
	if (_map_bounds_input_layout)
		_map_bounds_input_layout->Release();
}

void MapRenderer::DrawTile(const Tile& tile)
{
	auto offset = TILE_PIXEL_WIDTH / 2.0f;
	DrawSquare(
		XMFLOAT2(static_cast<float>(tile.x) + offset, static_cast<float>(tile.y) + offset),
		TILE_PIXEL_WIDTH,
		0.0f,
		0xFF0077FF);
}

void MapRenderer::DrawGrid()
{
	auto context = GraphicsWindow::GetInstance()->GetContext();

	context->IASetInputLayout(m_gridInputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	UINT stride = sizeof(WorldGrid::Vertex);
	UINT offset = 0;

	context->VSSetShader(m_gridShader.GetVertexShader(), nullptr, 0);
	context->PSSetShader(m_gridShader.GetPixelShader(), nullptr, 0);

	auto cameraBuffer = _cam->GetConstantBuffer();
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

void MapRenderer::DrawSquare(XMFLOAT2 position, float width, float rotation, unsigned color)
{
	auto context = GraphicsWindow::GetInstance()->GetContext();
	__declspec(align(16)) SquarePerObjectBuffer object{};
	object.color = ConvertColor(color);
	//auto translation_mat = XMMatrixTranslation(position.x, 0.0f, position.y);
	//auto scale_mat = XMMatrixScaling(width, 0.0f, width);
	//auto rotation_mat = XMMatrixRotationY(rotation);
	//auto world_mat = translation_mat * scale_mat;//translation_mat * rotation_mat * scale_mat;
	auto world_mat = XMMatrixIdentity() * 
					 XMMatrixScaling(width, 0.0f, width) * 
					 XMMatrixTranslation(position.x, 1.0f, position.y);

	XMStoreFloat4x4(&object.world_matrix, XMMatrixTranspose(world_mat));
	
	_UploadPerObjectBuffer(context, object);
	context->IASetInputLayout(m_squareInputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	UINT stride = sizeof(Square::Vertex);
	UINT offset = 0;

	context->VSSetShader(m_squareShader.GetVertexShader(), nullptr, 0);
	context->PSSetShader(m_squareShader.GetPixelShader(), nullptr, 0);

	auto cameraBuffer = _cam->GetConstantBuffer();
	context->VSSetConstantBuffers(0, 1, &cameraBuffer);
	context->VSSetConstantBuffers(1, 1, &m_perObjectBuffer);
	context->IASetVertexBuffers(0, 1, m_square.GetVertexBufferAddr(), &stride, &offset);

	auto vertex_count = m_square.GetVertexCount();
	context->Draw(vertex_count, 0);
}


void MapRenderer::DrawMapBounds()
{
	auto context = GraphicsWindow::GetInstance()->GetContext();
	context->IASetInputLayout(_map_bounds_input_layout); // same layout, might as well re-use
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	UINT stride = sizeof(Square::Vertex);
	UINT offset = 0;
	context->VSSetShader(_map_bounds_shader.GetVertexShader(), nullptr, 0);
	context->PSSetShader(_map_bounds_shader.GetPixelShader(), nullptr, 0);
	auto cameraBuffer = _cam->GetConstantBuffer();
	context->VSSetConstantBuffers(0, 1, &cameraBuffer);
	context->IASetVertexBuffers(0, 1, &_map_bounds_buffer, &stride, &offset);
	context->Draw(MAP_BOUNDS_VERTEX_COUNT, 0);
}
