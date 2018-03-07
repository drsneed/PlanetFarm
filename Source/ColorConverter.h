#pragma once
#include "StdIncludes.h"

inline XMFLOAT4 ConvertColor(unsigned color)
{
	return XMFLOAT4
	(
			(color >> 24 & 0xFF) / 255.f,
			(color >> 16 & 0xFF) / 255.f,
			(color >> 8 & 0xFF) / 255.f,
			(color & 0xFF) / 255.f
	);
}