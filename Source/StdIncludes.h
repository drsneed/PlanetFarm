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