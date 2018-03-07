#pragma once
#pragma once

#include "StdIncludes.h"
#include "GraphicsWindow.h"
#include <memory>

class Texture
{
public:
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	explicit Texture(const WCHAR* filename);
	Texture(int width, int height, int bitDepth, int componentCount, void* data);
	virtual ~Texture();

	auto Update(void* data) const -> void;

	ID3D11ShaderResourceView* GetShaderResourceView() const;
	void SetShaderResourceView(ID3D11ShaderResourceView* view);

	auto GetSize(int& width, int& height) -> void;
	auto GetSize(float& width, float& height) -> void;
	auto GetBitsPerPixel() const -> int;
	auto GetChannelCount() const -> int;

	explicit operator bool() const throw()
	{
		return m_resourceView != nullptr;
	}

	bool IsMonochrome();

protected:
	Texture(int width, int height, int bitsPerPixel, int channelCount);
	ID3D11ShaderResourceView* m_resourceView;
	int m_width;
	int m_height;
	int m_bitsPerPixel;
	int m_componentCount;

private:
	auto _DetermineFormat(int bitDepth, int componentCount)->DXGI_FORMAT;
	auto _ExpandRGBToRGBA(int width, int height, int bitDepth, void* data) -> void*;
	auto _LoadPNG(const WCHAR* filename) -> void;
	auto _LoadDDS(const WCHAR* filename) -> void;
	auto _CreateGPUTexture(void* data) -> void;
};

class RenderTexture : Texture
{
public:
	RenderTexture(int width, int height);
	~RenderTexture();

private:
	ID3D11RenderTargetView* m_renderTargetView;
};

extern std::shared_ptr<Texture> NullTexture;
