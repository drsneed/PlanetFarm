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
		ID3D11Buffer* vertex_buffer;
		int vertex_count;
		View() : vertex_buffer(nullptr), vertex_count(0) {}
		~View() 
		{ 
			if (vertex_buffer)
				vertex_buffer->Release();
		}

		View(const std::vector<XMFLOAT2>& vertex_data);

		// no copying
		View(View const&) = delete;
		View& operator=(View const&) = delete;

		// moving ok
		View(View&& other) noexcept
			: vertex_buffer(other.vertex_buffer)
			, vertex_count(other.vertex_count)
		{
			other.vertex_buffer = nullptr;
			other.vertex_count = 0;
		}

		inline View& operator=(View&& other) noexcept
		{
			if (this == &other)
				return *this;

			vertex_buffer = other.vertex_buffer;
			vertex_count = other.vertex_count;
			other.vertex_buffer = nullptr;
			other.vertex_count = 0;
			return *this;
		}

		void CreateBuffers(const std::vector<XMFLOAT2>& vertex_data);

	};


	TileID tile_id;
	XMFLOAT2 position;
	unsigned color;
	float rotation;
	
	DynamicFeature();
	DynamicFeature(Feature* feature, uint8_t zoom_level);
	~DynamicFeature();

	View* GetView(TileID tile_id);

	// no copying for now.
	DynamicFeature(DynamicFeature const&) = delete;
	DynamicFeature& operator=(DynamicFeature const&) = delete;

	// move ok
	DynamicFeature(DynamicFeature&& other) noexcept
		: _views(std::move(other._views))
		, position(other.position)
		, color(other.color)
		, rotation(other.rotation)
	{
	}

	inline DynamicFeature& operator=(DynamicFeature&& other) noexcept
	{
		if (this == &other)
			return *this;

		_views = std::move(other._views);
		position = other.position;
		color = other.color;
		rotation = other.rotation;
		return *this;
	}

private:
	std::mutex _views_mutex;
	std::map<TileID, View> _views;

};