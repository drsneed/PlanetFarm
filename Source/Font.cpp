#include "StdIncludes.h"
#include"Font.h"
#include "StringOps.h"

Font::Font(const char* fontDefinitionFile)
	: m_texture(nullptr)
	, m_lineHeight(0.0f)
	, m_baseLine(0.0f)
	, m_textureWidth(0.0f)
	, m_textureHeight(0.0f)
{
	tinyxml2::XMLDocument doc;

	if (doc.LoadFile(fontDefinitionFile) != tinyxml2::XMLError::XML_SUCCESS)
	{

	}

	auto root = doc.FirstChildElement("font");
	auto infoNode = root->FirstChildElement("info");
	auto commonNode = root->FirstChildElement("common");
	auto pages = root->FirstChildElement("pages");

	m_baseLine = commonNode->FloatAttribute("base");
	m_lineHeight = commonNode->FloatAttribute("lineHeight");
	m_textureWidth = commonNode->FloatAttribute("scaleW");
	m_textureHeight = commonNode->FloatAttribute("scaleH");

	_LoadTexture(pages);

	auto chars = root->FirstChildElement("chars");
	_FillCharMap(chars);

	auto kernings = root->FirstChildElement("kernings");
	_LoadKernings(kernings);
}

Font::~Font()
{
}

void Font::_PerformYAdjustment(vector<Vertex>& vertices)
{
	auto minY = FLT_MAX;
	for (auto& vertex : vertices)
	{
		if (vertex.Position.y < minY)
			minY = vertex.Position.y;
	}
	for (auto& vertex : vertices)
	{
		vertex.Position.y -= minY;
	}
}

void Font::_PerformYAdjustment(vector<TextRenderData>& chars)
{
	auto minY = FLT_MAX;
	for (auto& c : chars)
	{
		if (c.perObject.Position.y < minY)
			minY = c.perObject.Position.y;
	}
	for (auto& c : chars)
	{
		c.perObject.Position.y -= minY;
	}
}

auto Font::GetBaseLine()-> float
{
	return m_baseLine;
}

auto Font::GetLineHeight()-> float
{
	return m_lineHeight;
}

auto Font::GetDistanceFromTopToBaseLine(char c)-> float
{
	if (c == ' ')
	{
		return 0.0f;
	}
	return m_baseLine - m_glyphs[c].yoffset;
}

auto Font::GenerateRenderModel(const string& text)-> vector<TextRenderData>
{
	vector<TextRenderData> output;

	Pen pen(m_lineHeight, m_baseLine, m_textureWidth, m_textureHeight);
	TextRenderData model;
	GlyphRect rect;
	for (auto& ch : text)
	{
		if (pen.Write(m_glyphs[ch], rect))
		{
			model.texCoords[0] = rect.TopLeft.TexCoord;
			model.texCoords[1] = rect.BottomRight.TexCoord;
			model.texCoords[2] = rect.BottomLeft.TexCoord;
			model.texCoords[3] = rect.TopLeft.TexCoord;
			model.texCoords[4] = rect.TopRight.TexCoord;
			model.texCoords[5] = rect.BottomRight.TexCoord;
			model.perObject.Size.x = rect.BottomRight.Position.x - rect.BottomLeft.Position.x;
			model.perObject.Size.y = rect.BottomRight.Position.y - rect.TopRight.Position.y;
			model.perObject.Position = rect.TopLeft.Position;
			output.push_back(model);
		}
	}

	_PerformYAdjustment(output);

	return output;
}

auto Font::GetTexture() -> std::shared_ptr<Texture>
{
	return m_texture;
}

auto Font::GetStringSize(const string& text, float& width, float& height) -> void
{
	float maxX = 0.f, maxY = 0.f, minYOffset = FLT_MAX;
	Pen pen(m_lineHeight, m_baseLine, m_textureWidth, m_textureHeight);

	for (auto& ch : text)
	{
		GlyphRect rect;
		float x, yOffset, yHeight;
		if (pen.Advance(m_glyphs[ch], x, yOffset, yHeight))
		{
			float y = yOffset + yHeight;
			if (x > maxX)
			{
				maxX = x;
			}
			if (y > maxY)
			{
				maxY = y;
			}
			if (yOffset < minYOffset)
			{
				minYOffset = yOffset;
			}
		}
	}
	width = maxX;
	height = maxY - minYOffset;
}

void Font::_LoadTexture(tinyxml2::XMLElement* pagesElement)
{
	tinyxml2::XMLElement* pageElement = pagesElement->FirstChildElement("page");
	auto filename = string("Data/") + pageElement->Attribute("file");
	m_texture = std::make_shared<Texture>(StringOps::ToWideString(filename).c_str());
}

void Font::_LoadKernings(tinyxml2::XMLElement* kernings)
{
	if (!kernings)
	{
		return;
	}
	tinyxml2::XMLElement* kerningElement = kernings->FirstChildElement("kerning");
	int i = 0;
	do
	{
		// kerning value consists of "from" char (aka first), "to" char (aka second), amount 
		auto from = StringOps::UnicodeTo1252(
			static_cast<wchar_t>(kerningElement->IntAttribute("first")));
		auto to = StringOps::UnicodeTo1252(
			static_cast<wchar_t>(kerningElement->IntAttribute("second")));
		auto amount = kerningElement->IntAttribute("amount");
		m_glyphs[to].kernings.push_back(Kerning{ from,  amount });
		kerningElement = kerningElement->NextSiblingElement("kerning");
	} while (kerningElement);
}

auto Font::_FillCharMap(tinyxml2::XMLNode* charNode) -> bool
{
	tinyxml2::XMLElement* charElement = charNode->FirstChildElement("char");
	wstring wideCharacter(1, 0);
	Glyph glyph;
	do
	{
		wideCharacter[0] = static_cast<wchar_t>(charElement->IntAttribute("id"));
		auto character = StringOps::UnicodeTo1252(wideCharacter)[0];

		glyph.textureLocation.x = charElement->FloatAttribute("x");
		glyph.textureLocation.y = charElement->FloatAttribute("y");
		glyph.textureLocation.w = charElement->FloatAttribute("width");
		glyph.textureLocation.h = charElement->FloatAttribute("height");
		glyph.xoffset = charElement->FloatAttribute("xoffset");
		glyph.yoffset = charElement->FloatAttribute("yoffset");
		glyph.advance = charElement->FloatAttribute("xadvance");
		glyph.character = character;
		m_glyphs[character] = glyph;

		charElement = charElement->NextSiblingElement("char");
	} while (charElement);

	// Add whitespace characters.
	glyph.character = ' ';
	m_glyphs[' '] = glyph;
	glyph.character = '\n';
	m_glyphs['\n'] = glyph;
	glyph.character = '\t';
	m_glyphs['\t'] = glyph;
	return true;
}

auto Font::Pen::Write(const Glyph& glyph, GlyphRect& outRect) -> bool
{
	bool result = false;
	if (glyph.character == ' ')
	{
		x += 4.0f;
	}
	else if (glyph.character == '\n')
	{
		x = 0.0f;
		y += lineHeight;
	}
	else if (glyph.character == '\t')
	{
		x += 8.0f;
	}
	else
	{
		auto charX1 = x + glyph.xoffset;
		for (auto& kerning : glyph.kernings)
		{
			if (kerning.previousChar == last)
			{
				charX1 += kerning.amount;
			}
		}
		auto charY1 = y + glyph.yoffset;
		auto charX2 = charX1 + glyph.textureLocation.w;
		auto charY2 = charY1 + glyph.textureLocation.h;

		float u1, v1, u2, v2;
		u1 = glyph.textureLocation.x / textureWidth;
		v1 = glyph.textureLocation.y / textureHeight;
		u2 = (glyph.textureLocation.x + glyph.textureLocation.w) / textureWidth;
		v2 = (glyph.textureLocation.y + glyph.textureLocation.h) / textureHeight;
		// top left
		outRect.TopLeft.Position = XMFLOAT2(charX1, charY1);
		outRect.TopLeft.TexCoord = XMFLOAT2(u1, v1);
		outRect.TopRight.Position = XMFLOAT2(charX2, charY1);
		outRect.TopRight.TexCoord = XMFLOAT2(u2, v1);
		outRect.BottomLeft.Position = XMFLOAT2(charX1, charY2);
		outRect.BottomLeft.TexCoord = XMFLOAT2(u1, v2);
		outRect.BottomRight.Position = XMFLOAT2(charX2, charY2);
		outRect.BottomRight.TexCoord = XMFLOAT2(u2, v2);

		x += glyph.advance;
		result = true;
	}
	last = glyph.character;
	return result;
}

auto Font::Pen::Advance(const Glyph& glyph, float& maxX, float& yOffset, float& yHeight) -> bool
{
	if (glyph.character == ' ')
	{
		x += 4.0f;
		maxX = x;
		yOffset = 10.0f;
		yHeight = 10.0f;
		return true;
	}
	else if (glyph.character == '\t')
	{
		x += 8.0f;
		maxX = x;
		yOffset = y;
		yHeight = y;
		return true;
	}
	else if (glyph.character == '\n')
	{
		x = 0.0f;
		y += lineHeight;
		return false;
	}
	else
	{
		auto charX1 = x + glyph.xoffset;
		for (auto& kerning : glyph.kernings)
		{
			if (kerning.previousChar == last)
			{
				charX1 += kerning.amount;
			}
		}
		auto charY1 = y;// + glyph.yoffset;
		maxX = charX1 + glyph.textureLocation.w;
		//maxY = charY1 + glyph.textureLocation.h;
		yOffset = y + glyph.yoffset;
		yHeight = y + glyph.textureLocation.h;


		x += glyph.advance;
		return true;
	}
}