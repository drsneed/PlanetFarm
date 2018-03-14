#pragma once
#include "Camera.h"

class Map
{
	uint8_t _zoom_level;
	float _center;
public:
	Map();
	XMFLOAT2 GetCenter() { return XMFLOAT2(_center, _center); }
	/// Returns cursor position in world space
	XMFLOAT3 GetMouseCursorPosition(std::shared_ptr<Camera>& camera);
};