cbuffer PerFrameBuffer : register(b0)
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix ViewProjectionMatrix;
}

struct VertexInput
{
	float4 Position : POSITION;
	float3 Color : COLOR;
};

struct PixelInput
{
	float4 Position: SV_POSITION;
	float3 Color : COLOR;
};

PixelInput VS(VertexInput input)
{
	PixelInput output = (PixelInput)0;
	input.Position.w = 1.0f;
	output.Position = mul(input.Position, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);
	output.Color = input.Color;
	return output;
}

float4 PS(PixelInput input) : SV_TARGET
{
	return float4(input.Color.r, input.Color.g, input.Color.b, 1.0);
}