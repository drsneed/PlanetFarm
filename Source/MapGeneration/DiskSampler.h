#pragma once

#include <Core/StdIncludes.h>
#include "WidePoint.h"

namespace DiskSampler
{
	std::vector<WidePoint> GetSamples(uint32_t seed);
}
