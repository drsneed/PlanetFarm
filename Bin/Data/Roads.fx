cbuffer PerObjectBuffer : register(b0)
{
	float4 Color;
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
};

struct PixelInput
{
	float4 Position: SV_POSITION;
	float2 TexCoord: TEXCOORD0;
};

PixelInput VS(VertexInput input)
{
	PixelInput output = (PixelInput)0;
	float4 pos = float4(input.Position.x, input.Position.y, input.Position.z, 1.0f);
	output.Position = mul(pos, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);
	output.TexCoord = input.TexCoord;
	return output;
}

Texture2D Texture;
SamplerState Sampler;

float4 PS(PixelInput input) : SV_TARGET
{
	/*
	float3 normal = float3(0, 1, 0);
	float3 light = normalize(float3(0, 4, 1));
	float3 reflected = reflect(normalize(CameraPosition - input.Position.xyz), normal);
	float ambient = 0.05f;
	float nDotL = max(0, dot(normal, light));
	float3 diffuse = float3(DiffuseColor.r, DiffuseColor.g, DiffuseColor.b)* nDotL;

	float specular = pow(max(0, dot(reflected, light)), 64);

	float4 outputColor = float4(float3(ambient + 0.5 * diffuse), 1);
	*/
	float4 outputColor = Texture.Sample(Sampler, input.TexCoord) + Color;
	//float4 outputColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
	return outputColor;
}