#pragma once
#include <Core/StdIncludes.h>
#include <Core/GraphicsWindow.h>

class DynamicFeature;
class DynamicFeatureView
{
public:
	DynamicFeature* parent;
	ID3D11Buffer* vertex_buffer;
	int vertex_count;
	DynamicFeatureView() : vertex_buffer(nullptr), parent(nullptr), vertex_count(0) {}

	DynamicFeatureView(DynamicFeature* parent, const std::vector<XMFLOAT2>& vertex_data);

	DynamicFeatureView(DynamicFeatureView const& other)
	{
		vertex_buffer = other.vertex_buffer;
		vertex_count = other.vertex_count;
		parent = other.parent;
	}
	DynamicFeatureView& operator=(DynamicFeatureView const& other)
	{
		vertex_buffer = other.vertex_buffer;
		vertex_count = other.vertex_count;
		parent = other.parent;
		return *this;
	}

	bool operator==(const DynamicFeatureView& other)
	{
		return other.vertex_buffer == vertex_buffer;
	}

	// moving ok
	DynamicFeatureView(DynamicFeatureView&& other) noexcept
		: vertex_buffer(other.vertex_buffer)
		, vertex_count(other.vertex_count)
		, parent(other.parent)
	{
		other.parent = nullptr;
		other.vertex_buffer = nullptr;
		other.vertex_count = 0;
	}

	inline DynamicFeatureView& operator=(DynamicFeatureView&& other) noexcept
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