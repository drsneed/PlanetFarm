#include "TextRenderer.h"
#include "DebugTools.h"
#include "ColorConverter.h"

// topLeft, bottomRight, bottomLeft, topLeft, topRight, bottomRight

TextRenderer::TextRenderer(Font* defaultFont)
	: m_layout(nullptr)
	, m_perResizeBuffer(nullptr)
	, m_perObjectBuffer(nullptr)
	, m_vertexPositionBuffer(nullptr)
	, m_vertexTexCoordBuffer(nullptr)
	, m_textShader(LR"(Data\TextShader.fx)", Shader::Vertex | Shader::Pixel, "VS", "PS")
	, m_pipelinePrepared(false)
	, m_defaultFont(defaultFont)
{
	ASSERT(m_textShader);

	auto window = GraphicsWindow::GetInstance();
	auto device = window->GetDevice();
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	if (!D3DCheck(device->CreateInputLayout(
		layout,
		numElements,
		m_textShader.GetByteCode(Shader::Vertex)->GetBufferPointer(),
		m_textShader.GetByteCode(Shader::Vertex)->GetBufferSize(),
		&m_layout), L"ID3D11Device::CreateInputLayout (TextRenderer)")) return;

	m_textShader.ReleaseByteCode();

	// Create Constant Buffers in the shaders

	D3D11_BUFFER_DESC desc;

	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(CBPerScreenResize);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if(!D3DCheck(device->CreateBuffer(&desc, nullptr, &m_perResizeBuffer), 
		L"ID3D11Device::CreateBuffer (TextRenderer, CBPerScreenResize)")) return;

	desc.ByteWidth = sizeof(TextRenderDataCBPart);
	if(!D3DCheck(device->CreateBuffer(&desc, nullptr, &m_perObjectBuffer), 
		L"ID3D11Device::CreateBuffer (TextRenderer, UIConstantBuffer)")) return;

	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(XMFLOAT2) * 6;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if(!D3DCheck(device->CreateBuffer(&desc, nullptr, &m_vertexTexCoordBuffer), 
		L"ID3D11Device::CreateBuffer (TextRenderer, TexCoordBuffer")) return;

	// Initialize with a simple quad
	auto topLeft = XMFLOAT2(0.0f, 0.0f);
	auto topRight = XMFLOAT2(1.0f, 0.0f);
	auto bottomLeft = XMFLOAT2(0.0f, 1.0f);
	auto bottomRight = XMFLOAT2(1.0f, 1.0f);

	XMFLOAT2 vertices[] =
	{
		topLeft, bottomRight, bottomLeft, topLeft, topRight, bottomRight
	};

	// Create the vertex buffers
	ZeroMemory(&desc, sizeof(desc));

	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(XMFLOAT2) * 6;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	if(!D3DCheck(device->CreateBuffer(&desc, &InitData, &m_vertexPositionBuffer), 
		L"ID3D11Device::CreateBuffer (TextRenderer, VertexBuffer)")) return;

	window->GetSize(m_width, m_height);
	_UpdateResizeBuffer();

	m_changes.Reset();
}

void TextRenderer::_Render(const TextRenderData& renderModel)
{
	auto context = GraphicsWindow::GetInstance()->GetContext();

	// Upload per object buffer
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	if(!D3DCheck(context->Map(m_perObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes), 
		L"ID3D11DeviceContext::Map (TextRenderer::_Render perObject)")) return;
	memcpy_s(mappedRes.pData, mappedRes.RowPitch, &renderModel.perObject, sizeof(renderModel.perObject));
	context->Unmap(m_perObjectBuffer, 0);

	D3D11_MAPPED_SUBRESOURCE mappedRes2;
	if(!D3DCheck(context->Map(m_vertexTexCoordBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes2), 
		L"ID3D11DeviceContext::Map (TextRenderer::_Render texcoords)")) return;
	auto ptr = reinterpret_cast<XMFLOAT2*>(mappedRes2.pData);
	memcpy(ptr, renderModel.texCoords, sizeof(XMFLOAT2) * 6);
	context->Unmap(m_vertexTexCoordBuffer, 0);
	// Bind object texture
	auto resourceView = m_defaultFont->GetTexture()->GetShaderResourceView();
	context->PSSetShaderResources(0, 1, &resourceView);
	context->PSSetShader(m_textShader.GetPixelShader(), nullptr, 0);
	context->Draw(6, 0);
}

auto TextRenderer::PreparePipeline() -> bool
{
	if (m_pipelinePrepared)
		return true;
	auto window = GraphicsWindow::GetInstance();
	window->EnableAlphaBlending(true);

	auto context = window->GetContext();
	// Set the input layout
	context->IASetInputLayout(m_layout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT strides[] = { sizeof(XMFLOAT2), sizeof(XMFLOAT2) };
	UINT offsets[] = { 0, 0 };
	ID3D11Buffer* buffers[] = { m_vertexPositionBuffer, m_vertexTexCoordBuffer };
	context->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	UINT register_b0 = 0;
	UINT register_b1 = 1;
	context->VSSetConstantBuffers(register_b0, 1, &m_perResizeBuffer);
	context->VSSetConstantBuffers(register_b1, 1, &m_perObjectBuffer);

	// Set Vertex Shader
	context->VSSetShader(m_textShader.GetVertexShader(), nullptr, 0);

	// Set PS Sampler State
	auto samplerState = window->GetStandardSamplerState();
	context->PSSetSamplers(0, 1, &samplerState);

	m_pipelinePrepared = true;

	return false;
}

auto TextRenderer::RestorePipeline() -> void
{
	m_pipelinePrepared = false;
	GraphicsWindow::GetInstance()->EnableAlphaBlending(false);
}

TextRenderer::~TextRenderer()
{
	if(m_layout) m_layout->Release();
	if(m_perResizeBuffer) m_perResizeBuffer->Release();
	if(m_perObjectBuffer) m_perObjectBuffer->Release();
	if(m_vertexTexCoordBuffer) m_vertexTexCoordBuffer->Release();
	if(m_vertexPositionBuffer) m_vertexPositionBuffer->Release();
}

void TextRenderer::OnResize(int width, int height)
{
	m_width = width;
	m_height = height;
	_UpdateResizeBuffer();
}

void TextRenderer::_UpdateResizeBuffer()
{
	CBPerScreenResize localBuffer;
	localBuffer.ScreenSize = XMFLOAT2(static_cast<float>(m_width), static_cast<float>(m_height));
	// Update per resize buffer
	auto context = GraphicsWindow::GetInstance()->GetContext();
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	if(!D3DCheck(context->Map(m_perResizeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes), 
		L"D3D11DeviceContext::Map (TextRenderer::UpdateResizeBuffer)")) return;
	memcpy_s(mappedRes.pData, mappedRes.RowPitch, &localBuffer, sizeof(localBuffer));
	context->Unmap(m_perResizeBuffer, 0);
}

void TextRenderer::_UploadPerObjectBuffer(const TextRenderDataCBPart& perObject)
{
	// Update per resize buffer
	auto context = GraphicsWindow::GetInstance()->GetContext();
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	if(!D3DCheck(context->Map(m_perObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes), 
		L"ID3D11DeviceContext::Map (TextRenderer::UploadPerObjectBuffer)")) return;
	memcpy_s(mappedRes.pData, mappedRes.RowPitch, &perObject, sizeof(perObject));
	context->Unmap(m_perObjectBuffer, 0);
}

float TextRenderer::Printf(float x, float y, float depth, unsigned color, float scale, const char* text, ...)
{
	va_list args;
	va_start(args, text);
	char buf[256];
	vsnprintf_s(buf, 256, 256, text, args);
	va_end(args);
	auto inflatedColor = ConvertColor(color);
	auto baseLine = m_defaultFont->GetBaseLine();
	__declspec(align(16)) auto data = m_defaultFont->GenerateRenderModel(buf);
	for (auto& element : data)
	{
		element.perObject.Offset.x = x;
		element.perObject.Offset.y = y;
		element.perObject.Scale = scale;
		element.perObject.Color = inflatedColor;
		element.perObject.Depth = depth;
		_Render(element);
		depth -= DRAW_DEPTH_DECREMENT;
	}
	return depth;

}