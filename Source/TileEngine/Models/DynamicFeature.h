#pragma once
#include <Core/StdIncludes.h>
#include <map>
#include <mutex>
#include "DynamicFeatureView.h"
class Feature;
class DynamicFeature
{
public:

	typedef std::_Tree_const_iterator<std::_Tree_val<std::_Tree_simple_types<DynamicFeatureView>>> ViewIterator;

	TileID tile_id;
	XMFLOAT2 position;
	unsigned color;
	
	DynamicFeature();
	DynamicFeature(Feature* feature);
	~DynamicFeature();

	DynamicFeatureView GetView(TileID tile_id);
	DynamicFeatureView GetParentView(TileID tile_id);

	// no copying for now.
	DynamicFeature(DynamicFeature const&) = delete;
	DynamicFeature& operator=(DynamicFeature const&) = delete;

	// move ok
	DynamicFeature(DynamicFeature&& other) noexcept
		: _views(std::move(other._views))
		, position(other.position)
		, color(other.color)
		, tile_id(other.tile_id)
	{
		for (auto& view : _views)
		{
			view.second.parent = this;
		}
	}

	inline DynamicFeature& operator=(DynamicFeature&& other) noexcept
	{
		if (this == &other)
			return *this;

		_views = std::move(other._views);
		position = other.position;
		color = other.color;
		tile_id = other.tile_id;
		for (auto& view : _views)
		{
			view.second.parent = this;
		}
		return *this;
	}

private:
	void _BuildView(TileID tile_id);
	std::mutex _views_mutex;
	std::map<TileID, DynamicFeatureView> _views;
	//TileID _FindViewRecursive(TileID tile_id);

};