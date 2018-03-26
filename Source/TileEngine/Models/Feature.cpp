#include "Feature.h"

Feature::Feature()
	: _id(0)
	, _name()
	, _tile(INVALID_TILE_ID)
	, _type(FeatureType::Unknown)
	, _pos(0.0f, 0.0f)
	, _rot(0.0f)
	, _points()
{
	PRINTF(L"Feature Empty CTOR\n");
}

Feature::Feature(FeatureID id, const std::string& name, TileID tile_id, FeatureType type, XMFLOAT2 pos, float rot, const XMFLOAT2* points, size_t size_points)
	: _id(id)
	, _name(name)
	, _tile(tile_id)
	, _type(type)
	, _pos(pos)
	, _rot(rot)
	, _points(points, points + size_points)
{
	PRINTF(L"Feature CTOR(%d)\n", _id);
}

Feature::Feature(const std::string& name, TileID tile_id, FeatureType type, XMFLOAT2 pos, float rot, const std::vector<XMFLOAT2>& points)
	: _id(0)
	, _name(name)
	, _tile(tile_id)
	, _type(type)
	, _pos(pos)
	, _rot(rot)
	, _points(points)
{
	PRINTF(L"Feature CTOR(%d)\n", _id);
}