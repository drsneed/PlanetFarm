#include "Map.h"
#include <cmath>
#include <TileEngine/Tile.h>

namespace
{
	const float map_center_zoom_0 = (pow(2, TILE_MAX_ZOOM) * TILE_PIXEL_WIDTH) / 2.0f;
	const BoundingBox map_bbox_zoom_0
	{
		{ map_center_zoom_0, 0.0f, map_center_zoom_0 }, // Center of bbox
		{ map_center_zoom_0, 0.0f, map_center_zoom_0 } // Extents of bbox from center
	};
}

Map::Map(std::shared_ptr<Camera> camera)
	: _tile_engine(std::make_unique<TileEngine>())
	, _renderer(std::make_unique<MapRenderer>(camera))
	, _zoom(0, 0)
	, _cam(camera)
{
	_cam->NotifyPosChange(std::bind(&Map::HandleCameraPosChangedEvent, this, std::placeholders::_1));
}

void Map::HandleCameraPosChangedEvent(XMFLOAT3 position)
{
	GetCamCenter(true);
	_UpdateVisibleTiles();
}

void Map::_UpdateVisibleTiles()
{
	BoundingRect visible_area;
	visible_area.center = _cam_center;
	int width, height;
	GraphicsWindow::GetInstance()->GetSize(width, height);
	visible_area.extent.x = static_cast<float>(width) / 2.0f;
	visible_area.extent.y = static_cast<float>(height) / 2.0f;
	_visible_tiles = _tile_engine->Fetch(visible_area, _zoom.major_part);
}

void Map::ZoomIn()
{
	_zoom.inc();
}

void Map::ZoomOut()
{
	_zoom.dec();
}

void Map::ZoomTo(const ZoomLevel& level)
{
	_zoom = level;
}

auto Map::GetZoom() const -> ZoomLevel
{
	return _zoom;
}

MapPoint Map::GetCursor(bool refresh)
{
	if (!refresh)
		return _cursor;
	XMFLOAT3 origin, direction;
	_cam->ComputeRayFromMouseCursor(origin, direction);
	if (isnan(direction.x))
	{
		return _cursor;
	}
	XMVECTOR o = XMLoadFloat3(&origin);
	XMVECTOR d = XMLoadFloat3(&direction);
	float distance;
	if (map_bbox_zoom_0.Intersects(o, d, distance))
	{
		auto intersection_point = o + d * distance;
		XMFLOAT3 result;
		XMStoreFloat3(&result, intersection_point);
		_cursor = MapPoint(result.x, result.z);
	}
	return _cursor;
}

MapPoint Map::GetCamCenter(bool refresh)
{
	if (!refresh)
		return _cam_center;
	XMFLOAT3 origin, direction;
	_cam->ComputeRayFromScreenCenter(origin, direction);
	if (isnan(direction.x))
		return _cam_center;
	XMVECTOR o = XMLoadFloat3(&origin);
	XMVECTOR d = XMLoadFloat3(&direction);
	float distance;
	if (map_bbox_zoom_0.Intersects(o, d, distance))
	{
		auto intersection_point = o + d * distance;
		XMFLOAT3 result;
		XMStoreFloat3(&result, intersection_point);
		_cam_center = MapPoint(result.x, result.z);
	}
	return _cam_center;
}

void Map::Tick(float delta_time)
{
}

void Map::HandleEvent(const GraphicsWindow::Event & event)
{
	if (event.type == GraphicsWindow::Event::Type::MouseWheelDown)
	{
		ZoomOut();
	}
	else if (event.type == GraphicsWindow::Event::Type::MouseWheelUp)
	{
		ZoomIn();
	}
	else if (event.type == GraphicsWindow::Event::Type::MouseMotion)
	{
		GetCursor(true);
	}
}

void Map::RenderScene()
{
	for (auto& tile : _visible_tiles)
	{
		_renderer->DrawTile(tile);
	}
	_renderer->DrawMapBounds();
}