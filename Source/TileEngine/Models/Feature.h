#pragma once
#include <Core/StdIncludes.h>
#include <Core/GraphicsWindow.h>
#include "../Tile.h"
#include "StaticFeature.h"
#include "DynamicFeature.h"

//TODO: Make 'StaticFeature' and 'DynamicFeature' and 'CustomFeature' objects.
// These objects will be able to be rendered directly
// Static feature does not contain vertex buffer. Vertices are stored by renderer
// Dynamic feature contains its own vertex buffer that will be modified often.
// Think dynamically sized road, train tracks, crop, etc.
// CustomFeature contains its own static vertex buffer that cannot be modified after creation.
// Used for custom objects not known by the renderer in advance, but will not change often. E.g. a mountain range




typedef int64_t FeatureID;

enum class FeatureType : int
{
	Unknown,
	House,
	Office,
	Road
};

class Feature
{
	FeatureID _id;
	std::string _name;
	TileID _tile;
	FeatureType _type;
	XMFLOAT2 _pos;
	float _rot;
	std::vector<XMFLOAT2> _points;

public:
	Feature();
	Feature(FeatureID id, const std::string& name, TileID tile_id, FeatureType type, XMFLOAT2 pos, float rot, const XMFLOAT2* points, size_t size_points);
	Feature(const std::string& name, TileID tile_id, FeatureType type, XMFLOAT2 pos, float rot, const std::vector<XMFLOAT2>& points = std::vector<XMFLOAT2>());

	// no copying for now.
	Feature(Feature const&) = delete;
	Feature& operator=(Feature const&) = delete;

	// move ok
	Feature(Feature&& other) noexcept
		: _id(other._id)
		, _name(std::move(other._name))
		, _tile(other._tile)
		, _type(other._type)
		, _pos(other._pos)
		, _rot(other._rot)
		, _points(std::move(other._points))
	{
		other._id = 0;
		other._tile = INVALID_TILE_ID;
	}

	inline Feature& operator=(Feature&& other) noexcept
	{
		if (this == &other)
			return *this;

		PRINTF(L"Feature MOVE(old: %d, new: %d)\n", _id, other._id);

		_id = other._id;
		_name = std::move(other._name);
		_tile = other._tile;
		_type = other._type;
		_pos = other._pos;
		_rot = other._rot;
		_points = std::move(other._points);

		other._id = 0;
		other._tile = INVALID_TILE_ID;
		return *this;
	}

	XMFLOAT2 GetPosition() const { return _pos; }
	float GetRotation() const { return _rot; }
	XMFLOAT2 GetMapPosition() const { return Tile(_tile).GetFeaturePosition(_pos); }
	FeatureType GetType() const { return _type; }
	int GetTypeInt() const { return static_cast<int>(_type);  }
	TileID GetTileID() const { return _tile;  }
	FeatureID GetID() const { return _id;  }
	void SetID(FeatureID id) { ASSERT(_id == 0); _id = id; }
	std::vector<XMFLOAT2>& GetPointsRef() { return _points;  }
	bool HasPoints() const { return _points.size() > 0; }
	const std::string& GetName() const { return _name; }
};

