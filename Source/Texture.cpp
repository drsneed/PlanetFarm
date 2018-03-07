#include "StdIncludes.h"
#include "Texture.h"


#include <DirectXTex/DDSTextureLoader.h>
#include <DirectXTex/DirectXTex.h>
#include "Logger.h"
#include <stb_image.c>

std::shared_ptr<Texture> NullTexture;

Texture::Texture(const WCHAR* filename)
	: m_resourceView(nullptr)
	, m_width(0)
	, m_height(0)
	, m_bitsPerPixel(0)
	, m_componentCount(0)
{
	_LoadDDS(filename);
}

Texture::Texture(int width, int height, int bitDepth, int componentCount, void* data)
	: m_resourceView(nullptr)
	, m_width(width)
	, m_height(height)
	, m_bitsPerPixel(bitDepth * componentCount)
	, m_componentCount(componentCount)
{
	_CreateGPUTexture(data);
}

void Texture::Update(void* data) const
{
	ID3D11Resource* resource;
	m_resourceView->GetResource(&resource);
	auto context = GraphicsWindow::GetInstance()->GetContext();
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	if(!D3DCheck(context->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes),
		L"ID3D11DeviceContext::Map in Texture::Update")) return;
	auto size = m_width * m_height * (m_bitsPerPixel / 8) * m_componentCount;
	memcpy_s(mappedRes.pData, size, data, size);
	context->Unmap(resource, 0);
	resource->Release();
}

void Texture::GetSize(int& width, int& height)
{
	width = m_width;
	height = m_height;
}

auto Texture::GetSize(float& width, float& height)-> void
{
	width = static_cast<float>(m_width);
	height = static_cast<float>(m_height);
}

int Texture::GetBitsPerPixel() const
{
	return m_bitsPerPixel;
}

int Texture::GetChannelCount() const
{
	return m_componentCount;
}

Texture::~Texture()
{
	if (m_resourceView)
		m_resourceView->Release();
}

ID3D11ShaderResourceView* Texture::GetShaderResourceView() const
{
	return m_resourceView;
}

void Texture::SetShaderResourceView(ID3D11ShaderResourceView* view)
{
	if (m_resourceView)
		m_resourceView->Release();
	m_resourceView = view;
}

bool Texture::IsMonochrome()
{
	return m_componentCount == 1;
}

Texture::Texture(int width, int height, int bitsPerPixel, int channelCount)
	: m_resourceView(nullptr)
	, m_width(width)
	, m_height(height)
	, m_bitsPerPixel(bitsPerPixel)
	, m_componentCount(channelCount)
{
}

DXGI_FORMAT Texture::_DetermineFormat(int bitDepth, int componentCount)
{
	assert(componentCount > 0);
	assert(bitDepth > 0);
	switch (componentCount)
	{
	case 1:
		switch (bitDepth)
		{
		case 8: return DXGI_FORMAT_A8_UNORM;	// 8-BIT GRAYSCALE
		case 16: return DXGI_FORMAT_R16_UNORM;	// 16-BIT GRAYSCALE
		default: return DXGI_FORMAT_R32_UINT;
		}
	case 2:
		switch (bitDepth)
		{
		case 8: return DXGI_FORMAT_R8G8_UNORM;  // 8-BIT RG
		case 16: return DXGI_FORMAT_R16G16_UNORM; // 16-BIT RG
		default: return DXGI_FORMAT_R32G32_UINT; // 32-BIT RG
		}
	default: // treat as 4
		switch (bitDepth)
		{
		case 8: return DXGI_FORMAT_R8G8B8A8_UNORM;  // 8-BIT RG
		case 16: return DXGI_FORMAT_R16G16B16A16_UNORM; // 16-BIT RG
		default: return DXGI_FORMAT_R32G32B32A32_UINT; // 32-BIT RG
		}
	}
}

void* Texture::_ExpandRGBToRGBA(int width, int height, int bitDepth, void* data)
{
	switch (bitDepth)
	{
	case 8:
	{
		uint8_t* src = reinterpret_cast<uint8_t*>(data);
		uint8_t* mem = new uint8_t[width * height * 4];
		for (int i = 0; i < width * height; ++i)
		{
			mem[i * 4 + 0] = src[i * 3 + 0];
			mem[i * 4 + 1] = src[i * 3 + 1];
			mem[i * 4 + 2] = src[i * 3 + 2];
			mem[i * 4 + 3] = 0xFF;
		}
		return mem;
	}

	case 16:
	{
		uint16_t* src = reinterpret_cast<uint16_t*>(data);
		uint16_t* mem = new uint16_t[width * height * 4];
		for (int i = 0; i < width * height; ++i)
		{
			mem[i * 4 + 0] = src[i * 3 + 0];
			mem[i * 4 + 1] = src[i * 3 + 1];
			mem[i * 4 + 2] = src[i * 3 + 2];
			mem[i * 4 + 3] = 0xFF;
		}
		return mem;
	}
	default:
		return nullptr;
	}

}

auto Texture::_LoadPNG(const WCHAR* filename) -> void
{
	auto imgData = stbi_load(filename, &m_width, &m_height, &m_componentCount, 1);
	ASSERT(imgData);
	_CreateGPUTexture(imgData);
}

auto Texture::_LoadDDS(const WCHAR* filename) -> void
{
	DirectX::TexMetadata metadata;
	auto hr = GetMetadataFromDDSFile(filename, 0, metadata);
	if (FAILED(hr))
	{
		Logger::GetInstance()->WriteError(L"Failed to load '%s'. Reason: %s\n", filename, GetHRESULTErrorMessage(hr).c_str());
		D3DErrorOccurred();
		return;
	}

	m_width = static_cast<int>(metadata.width);
	m_height = static_cast<int>(metadata.height);
	m_bitsPerPixel = static_cast<int>(DirectX::BitsPerPixel(metadata.format));
	m_componentCount = m_bitsPerPixel / static_cast<int>(DirectX::BitsPerColor(metadata.format));

	hr = DirectX::CreateDDSTextureFromFile(GraphicsWindow::GetInstance()->GetDevice(), filename, nullptr, &m_resourceView);
	if (FAILED(hr))
	{
		Logger::GetInstance()->WriteError(L"Failed to load '%s'. Reason: %s\n", filename, GetHRESULTErrorMessage(hr));
		D3DErrorOccurred();
		return;
	}
}

auto Texture::_CreateGPUTexture(void* data)-> void
{
	ASSERT(data);

	auto converted = false;

	auto bitDepth = m_bitsPerPixel / m_componentCount;

	if (m_componentCount == 3)
	{
		data = reinterpret_cast<void*>(_ExpandRGBToRGBA(m_width, m_height, bitDepth, data));
		converted = true;
		m_componentCount = 4;
	}
	ASSERT(data);

	ID3D11Texture2D* tex;


	D3D11_SUBRESOURCE_DATA dataPtr;
	ZeroMemory(&dataPtr, sizeof(dataPtr));
	dataPtr.pSysMem = data;
	dataPtr.SysMemPitch = m_width * (bitDepth / 8) * m_componentCount;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = m_width;
	desc.Height = m_height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;

	// MSAA settings
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.Format = _DetermineFormat(bitDepth, m_componentCount);
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	auto device = GraphicsWindow::GetInstance()->GetDevice();
	if(!D3DCheck(device->CreateTexture2D(&desc, &dataPtr, &tex), L"ID3D11Device::CreateTexture2D")) return;

	if (converted)
	{
		delete[] data;
	}

	if (tex != nullptr)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		if(!D3DCheck(device->CreateShaderResourceView(tex, &srvDesc, &m_resourceView), L"ID3D11Device::CreateShaderResourceView")) return;
		tex->Release();
	}
}

RenderTexture::RenderTexture(int width, int height)
	: Texture(width, height, 128, 4)
{
	auto device = GraphicsWindow::GetInstance()->GetDevice();
	ID3D11Texture2D* texture;
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	if(!D3DCheck(device->CreateTexture2D(&textureDesc, nullptr, &texture), 
		L"ID3D11Device::CreateTexture2D")) return;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
	ZeroMemory(&renderTargetDesc, sizeof(renderTargetDesc));
	renderTargetDesc.Format = textureDesc.Format;
	renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetDesc.Texture2D.MipSlice = 0;

	if(!D3DCheck(device->CreateRenderTargetView(texture, &renderTargetDesc, &m_renderTargetView), 
		L"ID3D11Device::CreateRenderTargetView")) return;

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
	resourceViewDesc.Format = textureDesc.Format;
	resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MostDetailedMip = 0;
	resourceViewDesc.Texture2D.MipLevels = 1;
	if(!D3DCheck(device->CreateShaderResourceView(texture, &resourceViewDesc, &m_resourceView), 
		L"ID3D11Device::CreateShaderResourceView")) return;

	if (texture)
		texture->Release();

}

RenderTexture::~RenderTexture()
{
	if (m_resourceView)
		m_resourceView->Release();
	if (m_renderTargetView)
		m_renderTargetView->Release();
}