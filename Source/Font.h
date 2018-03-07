#pragma once
#include "StdIncludes.h"
#include "Texture.h"
#include <tinyxml2.h>
#include <map>

struct Kerning
{
	char previousChar;
	int amount;
};

struct Glyph
{
	char character;
	struct { float x; float y; float w; float h; } textureLocation;
	float xoffset, yoffset;
	float advance;
	vector<Kerning> kernings;
};

__declspec(align(16)) struct TextRenderDataCBPart
{
	XMFLOAT2 Position;
	XMFLOAT2 Size;
	XMFLOAT4 Color;
	float Depth;
	float Scale;
	XMFLOAT2 Offset;
};

struct TextRenderData
{
	TextRenderDataCBPart perObject;
	XMFLOAT2 texCoords[6];
};


class Font
{
public:
	Font(const char* fontDefinitionFile);
	~Font();

	struct Vertex
	{
		XMFLOAT2 Position;
		XMFLOAT2 TexCoord;
	};

	struct GlyphRect
	{
		Vertex TopLeft;
		Vertex TopRight;
		Vertex BottomLeft;
		Vertex BottomRight;
	};

	auto GetBaseLine() -> float;
	auto GetLineHeight() -> float;
	auto GetDistanceFromTopToBaseLine(char c) -> float;
	vector<TextRenderData> GenerateRenderModel(const string& text);
	std::shared_ptr<Texture> GetTexture();
	auto GetStringSize(const string& text, float& width, float& height) -> void;
private:

	struct Pen
	{
		float x;
		float y;
		char last;
		float lineHeight;
		float baseLine;
		float textureWidth;
		float textureHeight;
		Pen(float lineHeight, float baseLine, float textureWidth, float textureHeight)
			: x(0.0f), y(0.0f), last('\0'),
			lineHeight(lineHeight), baseLine(baseLine),
			textureWidth(textureWidth), textureHeight(textureHeight)
		{}

		bool Write(const Glyph& glyph, GlyphRect& outRect);
		bool Advance(const Glyph& glyph, float& maxX, float& yOffset, float& yHeight);

	};
	void _PerformYAdjustment(vector<Vertex>& vertices);
	void _PerformYAdjustment(vector<TextRenderData>& vertices);
	void _LoadKernings(tinyxml2::XMLElement* kernings);
	void _LoadTexture(tinyxml2::XMLElement* pageElement);
	auto _FillCharMap(tinyxml2::XMLNode* charNode) -> bool;
	std::map<char, Glyph> m_glyphs;
	std::shared_ptr<Texture> m_texture;
	float m_lineHeight;
	float m_baseLine;
	float m_textureWidth;
	float m_textureHeight;
};