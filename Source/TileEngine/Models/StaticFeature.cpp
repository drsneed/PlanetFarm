#include "Feature.h"
#include "StaticFeature.h"

StaticFeature::StaticFeature(const Feature* const feature)
	: model_id(0)
	, position(feature->GetMapPosition())
	, color(0x33FF33FF)
	, rotation(0.0f)
	, scale(1.0f)
{
}