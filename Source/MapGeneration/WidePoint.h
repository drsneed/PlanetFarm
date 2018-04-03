#pragma once

struct WidePoint
{
	static const std::size_t size = 2;
	typedef double value_type;
	double x;
	double y;
	double& operator[](std::size_t i) { if (i == 0) return x; return y; }
	const double& operator[](std::size_t i) const { if (i == 0) return x; return y; }

	XMFLOAT2 Shrink() { return XMFLOAT2(static_cast<float>(x), static_cast<float>(y)); }
};