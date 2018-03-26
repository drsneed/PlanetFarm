#pragma once
#include <Core/StdIncludes.h>

struct BoundingRect
{
	XMFLOAT2 center;
	XMFLOAT2 extent;

	void GetCorners(XMFLOAT2& top_left, XMFLOAT2& bottom_right)
	{
		top_left.x = center.x - extent.x;
		top_left.y = center.y + extent.y;
		bottom_right.x = center.x + extent.x;
		bottom_right.y = center.y - extent.y;
	}
};