cbuffer PerFrameBuffer : register(b0)
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix ViewProjectionMatrix;
}

struct VertexInput
{
	float2 Position : POSITION;
};

struct PixelInput
{
	float4 Position: SV_POSITION;
};

PixelInput VS(VertexInput input)
{
	PixelInput output = (PixelInput)0;
	float4 pos = float4(input.Position.x, 0.0, input.Position.y, 1.0);
	output.Position = mul(pos, ViewProjectionMatrix);
	return output;
}

float4 PS(PixelInput input) : SV_TARGET
{
	return float4(1,1,1,1);
}