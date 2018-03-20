#include "Map.h"
#include <cmath>
#include <TileEngine/Tile.h>

MapPoint InvalidMapPoint = XMFLOAT2(NAN, NAN);

bool IsValid(const MapPoint& map_point)
{
	return !isnan(map_point.x);
}
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
	GetCenterScreen(true);
}

void Map::HandleCameraPosChangedEvent(XMFLOAT3 position)
{
	GetCenterScreen(true);
	_UpdateVisibleTiles();
}

void Map::_UpdateVisibleTiles()
{
	if (isnan(_center_screen.x))
	{
		return;
	}
	BoundingRect visible_area;
	visible_area.center = _center_screen;
	int width, height;
	GraphicsWindow::GetInstance()->GetSize(width, height);
	auto top_right = ScreenPointToMapPoint(XMFLOAT2(width, 0.0f));
	visible_area.extent.x = top_right.x - _center_screen.x;
	visible_area.extent.y = top_right.y - _center_screen.y;
	_visible_tiles = _tile_engine->Fetch(visible_area, _zoom.major_part);
}

void Map::ZoomIn()
{
	_zoom.inc();
	_UpdateVisibleTiles();
}

void Map::ZoomOut()
{
	_zoom.dec();
	_UpdateVisibleTiles();
}

void Map::SetZoom(uint8_t major_part, uint8_t minor_part)
{
	_zoom = ZoomLevel(major_part, minor_part);
	_UpdateVisibleTiles();
}

auto Map::GetZoom() const -> ZoomLevel
{
	return _zoom;
}



MapPoint Map::ScreenPointToMapPoint(const XMFLOAT2& screen_point)
{
	XMFLOAT3 origin, direction;
	_cam->ComputeRayFromScreenPoint(screen_point, origin, direction);
	if (isnan(direction.x))
	{
		return InvalidMapPoint;
	}
	XMVECTOR o = XMLoadFloat3(&origin);
	XMVECTOR d = XMLoadFloat3(&direction);
	float distance;
	if (map_bbox_zoom_0.Intersects(o, d, distance))
	{
		auto intersection_point = o + d * distance;
		XMFLOAT3 result;
		XMStoreFloat3(&result, intersection_point);
		return MapPoint(result.x, result.z);
	}
	return InvalidMapPoint;
}

MapPoint Map::GetCursor(bool refresh)
{
	if (refresh)
	{
		auto screen_point = GraphicsWindow::GetInstance()->GetMousePosition();
		_cursor = ScreenPointToMapPoint(screen_point);
	}
	return _cursor;
}

MapPoint Map::GetCenterScreen(bool refresh)
{
	if (refresh)
	{
		int width, height;
		GraphicsWindow::GetInstance()->GetSize(width, height);
		_center_screen = ScreenPointToMapPoint(XMFLOAT2(width/2.0f, height/2.0f));
	}

	return _center_screen;
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
		_renderer->DrawTile(tile, 0xFFFF77FF);
		if (tile.Contains(_cursor))
		{
			auto pos = tile.GetPosition();
			_renderer->DrawSquare(pos.x, pos.y, 10.0f, 0.f, 0xFF0000FF);
		}
		
	}
	_renderer->DrawMapBounds();
}