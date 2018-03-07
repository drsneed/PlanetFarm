#pragma once
#pragma once
#include <Core/StdIncludes.h>
#include <Core/Texture.h>
#include <Core/Shader.h>
#include <Core/Font.h>

#define DRAW_DEPTH_DECREMENT 0.0001f
#define DRAW_DEPTH_START 0.5f

class TextRenderer
{
	// NO TextRenderer
	TextRenderer(const TextRenderer&) = delete;
	TextRenderer& operator=(const TextRenderer&) = delete;

public:
	TextRenderer(Font* defaultFont);
	~TextRenderer();

	void OnResize(int width, int height);
	float Printf(float x, float y, float depth, unsigned color, float scale, const char* text, ...);
	void Tick();
	struct Changes
	{
		bool WindowResize : 1;

		auto Reset() -> void
		{
			WindowResize = false;
		}
	};

	auto PreparePipeline() -> bool;
	auto RestorePipeline() -> void;

private:

	struct CBPerScreenResize
	{
		__declspec(align(16)) XMFLOAT2 ScreenSize;
	};

	__declspec(align(16)) struct PerObjectBuffer
	{
		XMFLOAT2 Position;
		XMFLOAT2 Size;
		XMFLOAT4 Color;
		float Depth;
		float Scale;
		XMFLOAT2 Offset;
	};

	struct RenderModel
	{
		PerObjectBuffer perObject;
		Texture* texture;
		XMFLOAT2 texCoords[6];
	};


	void _Render(const TextRenderData& renderModel);

	void _UpdateResizeBuffer();
	void _UploadPerObjectBuffer(const TextRenderDataCBPart& perObject);
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_perResizeBuffer;
	ID3D11Buffer* m_perObjectBuffer;
	ID3D11Buffer* m_vertexPositionBuffer;
	ID3D11Buffer* m_vertexTexCoordBuffer;
	Shader m_textShader;
	int m_width;
	int m_height;
	Changes m_changes;
	bool m_pipelinePrepared;
	Font* m_defaultFont;
};
