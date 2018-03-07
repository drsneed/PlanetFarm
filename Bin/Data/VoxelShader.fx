cbuffer VoxelInstanceBuffer : register(b0)
{
	float4 ChunkPosition;
	float3 Color;
}

cbuffer CameraBuffer : register(b1)
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix ViewProjectionMatrix;
	matrix ViewportMatrix;
	float3 CameraPosition;
	float padding;
}

struct VertexInput
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL0;
};

struct PixelInput
{
	float4 Position: SV_POSITION;
	float2 TexCoord: TEXCOORD0;
	float3 Normal : NORMAL0;
};

PixelInput VS(VertexInput input)
{
	PixelInput output = (PixelInput)0;
	float4 pos = float4(input.Position + ChunkPosition.xyz, 1.0);
	output.Position = mul(pos, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);
	output.TexCoord = input.TexCoord;
	output.Normal = input.Normal;
	return output;
}

/*Texture2D Texture;
SamplerState Sampler;*/

float4 PS(PixelInput input) : SV_TARGET
{
	
	float3 light = normalize(float3(6, 4, 6));
	float3 reflected = reflect(normalize(CameraPosition - input.Position.xyz), input.Normal);
	float ambient = 0.05f;
	float nDotL = max(0, dot(input.Normal, light));
	float3 diffuse = float3(Color.r, Color.g, Color.b)* nDotL;

	float specular = pow(max(0, dot(reflected, light)), 64);

	float4 outputColor = float4(float3(ambient + 0.5 * diffuse), 1);

	return outputColor;
}