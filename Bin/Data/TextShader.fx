
cbuffer CBPerFrame : register(b0)
{
	float2 ScreenSize;
};

cbuffer CBPerObject : register(b1)
{
	float2 Position;
	float2 Size;
	float4 Color;
	float Depth;
	float Scale;
	float2 Offset;
};

Texture2D Texture;
SamplerState Sampler;


struct VSInput
{
	float2 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
};

struct PSInput
{
	float4 Position : SV_POSITION;
	float4 Color : TEXCOORD0;
	float2 TexCoord : TEXCOORD1;

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
	float2 position = input.Position * Size;
	position.x += Position.x;
	position.y += Position.y;
	position *= Scale;
	position += Offset;
	position = ToScreenSpace(position);
	
	output.Position = float4(position.x, position.y, Depth, 1.0);
	output.Color = Color;
	output.TexCoord = input.TexCoord;
	return output;
}

float4 PS(PSInput input) : SV_TARGET
{
	float4 color = Texture.Sample(Sampler, input.TexCoord);
	float4 returnValue = input.Color * color.a;
	return returnValue;
}



