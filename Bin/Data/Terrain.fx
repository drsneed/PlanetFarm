#define USHORT_SIZE 65535.0f
#define MAX_QUADTREE_LEVEL_COUNT 15
#define HEIGHTMAP_SIZE 4096.0f
#define ONE_OVER_HEIGHTMAP_SIZE 0.000244140625f

cbuffer PerObjectBuffer : register(b0)
{
	matrix WorldMatrix;
	float4 DiffuseColor;
	float2 MorphConst;
	float CurrentNodeSize;
	float CurrentNodeLevel;
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

cbuffer ConstantBuffer : register(b2)
{
	float LeafNodeSize;
	float HeightScale;
	float DrawGridOverlay;
	float padding2;
}


Texture2D Heightmap;
SamplerState Sampler;

struct VertexInput
{
	float2 Position : POSITION;
};

struct GeometryInput
{
	float4 Position: SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 VertexPosition : TEXCOORD1;
};

struct PixelInput
{
	float4 Position: SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 VertexPosition : TEXCOORD1;
	float3 EdgeDistance : TEXCOORD2;
};

float2 MorphVertex(float2 gridPosition, float2 vertex, float morphK)
{
	float2 fracPart = frac(gridPosition * LeafNodeSize * 0.5) * (2.0 / LeafNodeSize);
	return vertex - (fracPart * morphK * CurrentNodeSize); 
}

GeometryInput VS(VertexInput input)
{
	GeometryInput output = (GeometryInput)0;
	float4 pos = float4(input.Position.x, 0.0f, input.Position.y, 1.0f);
	output.Position = mul(pos, WorldMatrix);
	//output.TexCoord = float2(output.Position.x / HEIGHTMAP_SIZE, 1.0 - (output.Position.z / HEIGHTMAP_SIZE));
	output.TexCoord = float2(output.Position.x / HEIGHTMAP_SIZE, output.Position.z / HEIGHTMAP_SIZE);
	float height = Heightmap.SampleLevel(Sampler, output.TexCoord, 0).r * USHORT_SIZE;
	output.Position.y = height * HeightScale;
	float morphLerpK = 1.0f - saturate(MorphConst.x - distance(CameraPosition, output.Position.xyz) * MorphConst.y);

	output.Position.xz = MorphVertex(input.Position / LeafNodeSize, output.Position.xz, morphLerpK);

	//output.Position may have changed. Recalculate texcoords
	//output.TexCoord = float2(output.Position.x / HEIGHTMAP_SIZE, 1.0 - (output.Position.z / HEIGHTMAP_SIZE));
	output.TexCoord = float2(output.Position.x / HEIGHTMAP_SIZE, output.Position.z / HEIGHTMAP_SIZE);
	// tex coords might have changed. Need to find height at new location 
	height = Heightmap.SampleLevel(Sampler, output.TexCoord, 0).r * USHORT_SIZE;

	output.Position.y = height * HeightScale;
	output.VertexPosition = output.Position.xyz;
	output.Position = mul(output.Position, ViewMatrix);
	output.Position = mul(output.Position, ProjectionMatrix);
	return output;
}


void GetEdgeDistances(GeometryInput tri[3], out float3 values[3])
{
	// Algorithm taken from OpenGL Shading Language Cookbook 4.0
	// perform the perspective divide and put into viewport coordinates
	float3 p0 = mul(tri[0].Position / tri[0].Position.w, ViewportMatrix).xyz;
	float3 p1 = mul(tri[1].Position / tri[1].Position.w, ViewportMatrix).xyz;
	float3 p2 = mul(tri[2].Position / tri[2].Position.w, ViewportMatrix).xyz;

	// find the altitudes (ha, hb and hc)
	float a = length(p1 - p2);
	float b = length(p2 - p0);
	float c = length(p1 - p0);

	// Use the law of cosines
	float alpha = acos((b*b + c*c - a*a) / (2.0 * b * c));
	float beta = acos((a*a + c*c - b*b) / (2.0 * a * c));

	float ha = abs(c * sin(beta));
	float hb = abs(c * sin(alpha));
	float hc = abs(b * sin(alpha));

	values[0] = float3(ha, 0, 0);
	values[1] = float3(0, hb, 0);
	values[2] = float3(0, 0, hc);
}

[maxvertexcount(3)]
void GS(
	triangle GeometryInput input[3],
	inout TriangleStream<PixelInput> output)
{
	float3 edgedist[3];
	GetEdgeDistances(input, edgedist);

	[unroll]
	for (int i = 0; i < 3; ++i)
	{
		PixelInput result;
		result.EdgeDistance = edgedist[i];
		result.Position = input[i].Position;
		result.VertexPosition = input[i].VertexPosition;
		result.TexCoord = input[i].TexCoord;
		output.Append(result);
	}
}

/*cbuffer LightBuffer
{
	float4 Diffuse;
	float3 LightDirection;
	float padding;
}

Texture2D GridTexture;
SamplerState Sampler;
*/



float3 GetNormal(float2 tc)
{
	  const float3 off = float3(ONE_OVER_HEIGHTMAP_SIZE, ONE_OVER_HEIGHTMAP_SIZE, 0.0f);
	  float hL = Heightmap.Sample(Sampler, tc - off.xz).r * USHORT_SIZE * HeightScale;
	  float hR = Heightmap.Sample(Sampler, tc + off.xz).r * USHORT_SIZE * HeightScale;
	  float hD = Heightmap.Sample(Sampler, tc - off.zy).r * USHORT_SIZE * HeightScale;
	  float hU = Heightmap.Sample(Sampler, tc + off.zy).r * USHORT_SIZE * HeightScale;
	  return normalize(float3(hL-hR, 2.0f, hD-hU));
}

#define LINE_WIDTH 0.75f
#define GRID_VISIBILITY_RADIUS 50.f
static const float4 GridColor = float4(1.0f, 0.0f, 0.0f, 1.0f);

float4 PS(PixelInput input) : SV_TARGET
{
	float3 normal = GetNormal(input.TexCoord);
	float3 light = normalize(float3(0,4,1));
    float3 reflected = reflect(normalize(CameraPosition-input.Position.xyz), normal);
    float ambient = 0.05f;
    float nDotL = max(0,dot(normal, light));
	float3 diffuse = float3(DiffuseColor.r, DiffuseColor.g, DiffuseColor.b)* nDotL;
    
    float specular = pow(max(0, dot(reflected, light)), 64);
	//FinalColor = vec4(float3(ambient + 0.5 * diffuse + 0.4 * specular), 1);

	float4 outputColor = float4(float3(ambient + 0.5 * diffuse), 1);

	float closest = min(input.EdgeDistance.y, input.EdgeDistance.z); // just draws the grid, doesn't work anymore for some reason
	//float closest = min(min(input.EdgeDistance.x, input.EdgeDistance.y), input.EdgeDistance.z); // draws all the edges
	float mixRatio = smoothstep(LINE_WIDTH - 1.0f, LINE_WIDTH + 1.0f, closest) * DrawGridOverlay;
	
	float dist = distance(input.VertexPosition, CameraPosition);
	if(dist < GRID_VISIBILITY_RADIUS)
		outputColor = lerp(outputColor, GridColor, ((1.0f*DrawGridOverlay)) - mixRatio);
	
	return outputColor;
	//return float4(GetNormal(input.TexCoord), 1);
}