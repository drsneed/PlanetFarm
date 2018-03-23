#include "Feature.h"

Feature::Feature()
	: _id(0)
	, _name()
	, _tile(INVALID_TILE_ID)
	, _type(FeatureType::Unknown)
	, _pos(0.0f, 0.0f)
	, _points()
{

}

Feature::Feature(FeatureID id, const std::string& name, TileID tile_id, FeatureType type, XMFLOAT2 pos, const XMFLOAT2* points, size_t size_points)
	: _id(id)
	, _name(name)
	, _tile(tile_id)
	, _type(type)
	, _pos(pos)
	, _points(points, points + size_points)
{
}

Feature::Feature(const std::string& name, TileID tile_id, FeatureType type, XMFLOAT2 pos, const std::vector<XMFLOAT2>& points)
	: _id(0)
	, _name(name)
	, _tile(tile_id)
	, _type(type)
	, _pos(pos)
	, _points(points)
{
}