// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#include <vector>
#include <string>

using std::vector;
using std::string;
using std::wstring;

#include <DirectXMath.h>
#include <DirectXCollision.h>
using namespace DirectX;

#include <memory>
#include <random>
#include <ctime>

typedef XMFLOAT2 MapPoint;

#define EPSILON 0.0001f

inline bool EqualFloats(float a, float b)
{
	return fabs(a - b) < EPSILON;
}

inline float div2(float a, uint8_t n)
{
	for (uint8_t i = 0; i < n; ++i)
	{
		a /= 2.0f;
	}
	return a;
}