
cbuffer CBPerFrame : register(b0)
{
	float2 ScreenSize;
};

cbuffer CBPerObject : register(b1)
{
	float2 Scale;
	float2 Position;
	float4 Color;
};

Texture2D FontTexture;
SamplerState Sampler;


struct VSInput
{
	float2 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
	float Depth     : TEXCOORD1;
};

struct PSInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
};

float2 ToScreenSpace(float2 input)
{
	float2 halfSize = ScreenSize / 2.0;
	input.x = (input.x - halfSize.x) / halfSize.x;
	input.y = -((input.y - halfSize.y) / halfSize.y);
	return input;
}

PSInput VS(VSInput input)
{
	PSInput output;
	float2 position = input.Position;
	position *= Scale;
	position.y += Position.y;
	position.x += Position.x;

	position = ToScreenSpace(position);

	output.Position = float4(position.x, position.y, input.Depth, 1.0);
	output.TexCoord = input.TexCoord;
	return output;
}

float4 PS(PSInput input) : SV_TARGET
{
	return Color * FontTexture.Sample(Sampler, input.TexCoord).a * Color.a;
}


