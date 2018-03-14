cbuffer PerFrameBuffer : register(b0)
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix ViewProjectionMatrix;
}

cbuffer PerObject : register(b1)
{
	matrix WorldMatrix;
	float4 Color;
}

struct VertexInput
{
	float2 Position : POSITION;
};

struct PixelInput
{
	float4 Position: SV_POSITION;
	float4 Color : COLOR;
};

PixelInput VS(VertexInput input)
{
	PixelInput output = (PixelInput)0;
	float4 pos = float4(input.Position.x, 0.0, input.Position.y, 1.0);
	output.Position = mul(pos, WorldMatrix);
	output.Position = mul(output.Position, ViewProjectionMatrix);
	output.Color = Color;
	
	return output;
}

float4 PS(PixelInput input) : SV_TARGET
{
	return input.Color;
}