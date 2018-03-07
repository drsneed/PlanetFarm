
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
	return input.Color * input.Color.a;
}

#define ROUND_DIST_MIN 0.04f
#define ROUND_DIST_MAX (1.0f - ROUND_DIST_MIN)
#define ROUND_AA 0.004f
float4 PS_Beveled(PSInput input) : SV_TARGET
{
	float2 roundDistMin = (Size * ROUND_DIST_MIN) / Size;
	float2 roundDistMax = (Size * ROUND_DIST_MAX) / Size;
	float alpha = input.Color.a;
/*	if (input.TexCoord.x < roundDistMin.x && input.TexCoord.y < roundDistMin.y)
	{
		float dist = length(roundDistMin - input.TexCoord);
		
		alpha = lerp(1.0f, 0.0f, smoothstep(ROUND_DIST_MIN, ROUND_DIST_MIN + ROUND_AA, dist));
	}
	else if (input.TexCoord.x > roundDistMax.x && input.TexCoord.y < roundDistMin.y)
	{
		float dist = length(float2(roundDistMax.x, roundDistMin.y) - input.TexCoord);

		alpha = lerp(1.0f, 0.0f, smoothstep(ROUND_DIST_MIN, ROUND_DIST_MIN + ROUND_AA, dist));
	}
	
	else if (input.TexCoord.x > roundDistMax.x && input.TexCoord.y > roundDistMax.y)
	{
		float dist = length(roundDistMax - input.TexCoord);

		alpha = lerp(1.0f, 0.0f, smoothstep(ROUND_DIST_MIN, ROUND_DIST_MIN + ROUND_AA, dist));
	}

	else if (input.TexCoord.x < roundDistMin.x && input.TexCoord.y > roundDistMax.y)
	{
		float dist = length(float2(roundDistMin.x, roundDistMax.y) - input.TexCoord);

		alpha = lerp(1.0f, 0.0f, smoothstep(ROUND_DIST_MIN, ROUND_DIST_MIN + ROUND_AA, dist));
	}*/
	
	return input.Color * alpha;
}


float4 PS_NO_SDF(PSInput input) : SV_TARGET
{
	return input.Color * Texture.Sample(Sampler, input.TexCoord).a * input.Color.a;
	//return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

float4 PS_SDF(PSInput input) : SV_TARGET
{
	/*
	const float smoothing = 0.03f;
	float distance = Texture.Sample(Sampler, input.TexCoord).a;
	float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance) * input.Color.a;
	return float4(input.Color.rgb, alpha);
	*/

	/*
	float edgeDistance = 0.5;
	float dist = Texture.Sample(Sampler, input.TexCoord).a;
	float edgeWidth = 0.7 * length(float2(ddx(dist), ddy(dist)));
	float opacity = smoothstep(edgeDistance - edgeWidth, edgeDistance + edgeWidth, dist);
	return float4(input.Color.r, input.Color.g, input.Color.b, opacity) * input.Color.a;
	*/

	
	// sample distance field, transform from [0..1] to [-1..1]
	float d = Texture.Sample(Sampler, input.TexCoord).a * 2.0f - 1.0f;
	// Perform anisotropic analytic antialiasing
	float aastep = length(float2(ddx(d), ddy(d)));
	// calculate alpha value
	float alpha = smoothstep(-aastep, aastep, d);
	// return new color
	return float4(input.Color.rgb, alpha) * input.Color.a;
}

float4 PS_RGBA(PSInput input) : SV_TARGET
{
	float4 color = Texture.Sample(Sampler, input.TexCoord);
	float4 returnValue =  color * color.a;
	return returnValue;
}



