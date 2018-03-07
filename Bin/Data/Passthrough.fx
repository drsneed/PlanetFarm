cbuffer PerObjectBuffer : register(b0)
{
	float4 Color;
}

cbuffer PerFrameBuffer : register(b1)
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix ViewProjectionMatrix;
}

struct VertexInput
{
	float3 Position : POSITION;
};

struct PixelInput
{
	float4 Position: SV_POSITION;
};

PixelInput VS(VertexInput input)
{
	PixelInput output = (PixelInput)0;
	float4 p = float4(input.Position, 1.0f);
	output.Position = mul(p, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);
	return output;
}

float4 PS(PixelInput input) : SV_TARGET
{
	return Color;
}