#pragma once
#include <Core/StdIncludes.h>
#include <Core/GraphicsWindow.h>

class Feature;
struct DynamicFeature
{
	ID3D11Buffer* vertex_buffer;
	int vertex_count;

	XMFLOAT2 position;
	unsigned color;
	float rotation;
	float scale;

	DynamicFeature(Feature* feature, uint8_t zoom_level);

	~DynamicFeature();


	// no copying for now.
	DynamicFeature(DynamicFeature const&) = delete;
	DynamicFeature& operator=(DynamicFeature const&) = delete;

	// move ok
	DynamicFeature(DynamicFeature&& other) noexcept
		: vertex_buffer(other.vertex_buffer)
		, vertex_count(other.vertex_count)
		, position(other.position)
		, color(other.color)
		, rotation(other.rotation)
		, scale(other.scale)
	{
		other.vertex_buffer = nullptr;
		other.vertex_count = 0;
	}

	inline DynamicFeature& operator=(DynamicFeature&& other) noexcept
	{
		if (this == &other)
			return *this;

		vertex_buffer = other.vertex_buffer;
		vertex_count = other.vertex_count;
		position = other.position;
		color = other.color;
		rotation = other.rotation;
		scale = other.scale;

		other.vertex_buffer = nullptr;
		other.vertex_count = 0;
		return *this;
	}

};