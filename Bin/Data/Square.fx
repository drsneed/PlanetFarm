cbuffer PerFrameBuffer : register(b0)
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix ViewProjectionMatrix;
}

cbuffer PerObject : register(b1)
{
	float4 ObjectColor;
	float2 ObjectPosition;
	float2 ObjectSize;
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
	//float4 pos = ((input.Position.x * ObjectSize.x) + ObjectPosition.x, 0.0, 
		//(input.Position.y * ObjectSize.y) + ObjectPosition.y, 1.0);
	float4 pos = float4(input.Position.x * ObjectSize.x, 0.0, input.Position.y * ObjectSize.y, 1.0);
	output.Position = mul(pos, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);
	output.Color = ObjectColor;
	
	return output;
}

float4 PS(PixelInput input) : SV_TARGET
{
	return input.Color;
}