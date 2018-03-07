cbuffer PerObjectBuffer : register(b0)
{
	matrix WorldMatrix;
}

cbuffer PerFrameBuffer : register(b1)
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix ViewProjectionMatrix;
}

struct VertexInput
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;
};

struct PixelInput
{
	float4 Position: SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;

};

PixelInput VS(VertexInput input)
{
	PixelInput output = (PixelInput)0;
	input.Position.w = 1.0f;
	output.Position = mul(input.Position, WorldMatrix);
	output.Position = mul(output.Position, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);
	output.TexCoord = input.TexCoord;
	output.Normal = normalize(output.Normal);
	return output;
}

/*cbuffer LightBuffer
{
	float4 Diffuse;
	float3 LightDirection;
	float padding;
}*/

Texture2D DiffuseTexture;
SamplerState Sampler;

float4 PS(PixelInput input) : SV_TARGET
{
	return DiffuseTexture.Sample(Sampler, input.TexCoord);
}