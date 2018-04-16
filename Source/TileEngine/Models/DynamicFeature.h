#pragma once
#include <Core/StdIncludes.h>
#include <Core/GraphicsWindow.h>
#include <map>
#include <mutex>

class Feature;
struct DynamicFeature
{
	struct View
	{
		DynamicFeature* parent;
		ID3D11Buffer* vertex_buffer;
		int vertex_count;
		View() : vertex_buffer(nullptr), parent(nullptr), vertex_count(0) {}

		View(DynamicFeature* parent, const std::vector<XMFLOAT2>& vertex_data);

		View(View const& other)
		{ 
			vertex_buffer = other.vertex_buffer;
			vertex_count = other.vertex_count;
			parent = other.parent;
		}
		View& operator=(View const& other)
		{
			vertex_buffer = other.vertex_buffer;
			vertex_count = other.vertex_count;
			parent = other.parent;
			return *this;
		}

		bool operator==(const View& other)
		{
			return other.vertex_buffer == vertex_buffer;
		}

		// moving ok
		View(View&& other) noexcept
			: vertex_buffer(other.vertex_buffer)
			, vertex_count(other.vertex_count)
			, parent(other.parent)
		{
			other.parent = nullptr;
			other.vertex_buffer = nullptr;
			other.vertex_count = 0;
		}

		inline View& operator=(View&& other) noexcept
		{
			if (this == &other)
				return *this;

			vertex_buffer = other.vertex_buffer;
			vertex_count = other.vertex_count;
			parent = other.parent;
			other.parent = nullptr;
			other.vertex_buffer = nullptr;
			other.vertex_count = 0;
			return *this;
		}

		void CreateBuffers(const std::vector<XMFLOAT2>& vertex_data);

	};

	typedef std::_Tree_const_iterator<std::_Tree_val<std::_Tree_simple_types<DynamicFeature::View>>> ViewIterator;

	TileID tile_id;
	XMFLOAT2 position;
	unsigned color;
	
	DynamicFeature();
	DynamicFeature(Feature* feature);
	~DynamicFeature();

	View GetView(TileID tile_id);

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
	std::mutex _views_mutex;
	std::map<TileID, View> _views;
	//TileID _FindViewRecursive(TileID tile_id);

};