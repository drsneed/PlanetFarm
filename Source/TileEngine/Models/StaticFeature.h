#pragma once
#include <Core/StdIncludes.h>
class Feature;
struct StaticFeature
{
	int model_id;
	XMFLOAT2 position;
	unsigned color;
	float rotation;
	float scale;

	StaticFeature(const Feature* const feature);
};