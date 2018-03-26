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
	const BoundingBox map_bbox_zoom_0
	{
		{ MAP_ABSOLUTE_CENTER, 0.0f, MAP_ABSOLUTE_CENTER }, // Center of bbox
		{ MAP_ABSOLUTE_CENTER, 0.0f, MAP_ABSOLUTE_CENTER } // Extents of bbox from center
	};
}

Map::Map(std::shared_ptr<Camera> camera, const char* const db_filename)
	: _tile_engine(std::make_unique<TileEngine>(db_filename))
	, _renderer(std::make_unique<MapRenderer>(camera))
	, _zoom(0, 0)
	, _cam(camera)
	, _visible_tiles_frozen(false)
{
	_cam->NotifyPosChange(std::bind(&Map::HandleCameraPosChangedEvent, this, std::placeholders::_1));
}

void Map::HandleCameraPosChangedEvent(XMFLOAT3 position)
{
	GetCenterScreen(true);
	_RefreshTiles();
}

void Map::_RefreshTiles()
{
	if (isnan(_center_screen.x) || _visible_tiles_frozen)
	{
		return;
	}

	_visible_area.center = _center_screen;
	int width, height;
	GraphicsWindow::GetInstance()->GetSize(width, height);
	// offset by TILE_PIXEL_WIDTH so the visible_area has a 1 tile buffer outside of the visible area
	auto top_right = ScreenPointToMapPoint(XMFLOAT2(width + TILE_PIXEL_WIDTH, -TILE_PIXEL_WIDTH));
	_visible_area.extent.x = top_right.x - _center_screen.x;
	_visible_area.extent.y = top_right.y - _center_screen.y;

	_tile_engine->Refresh(_visible_area, _zoom.major_part);
}

void Map::ZoomIn()
{
	_zoom.inc();
	_RefreshTiles();
}

void Map::ZoomOut()
{
	_zoom.dec();
	_RefreshTiles();
}

void Map::SetZoom(uint8_t major_part, uint8_t minor_part)
{
	if (_zoom.major_part == major_part && _zoom.minor_part == minor_part)
		return;
	_zoom = ZoomLevel(major_part, minor_part);
	_RefreshTiles();
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

typedef GraphicsWindow::Event::Type EventType;

void Map::HandleEvent(const GraphicsWindow::Event & event)
{


	if (event.type == EventType::MouseWheelDown)
	{
		ZoomOut();
	}
	else if (event.type == EventType::MouseWheelUp)
	{
		ZoomIn();
	}
	else if (event.type == EventType::MouseMotion)
	{
		GetCursor(true);
	}

	if (event.code == GraphicsWindow::Event::Code::F)
	{
		if (_visible_tiles_frozen)
		{
			_visible_tiles_frozen = false;
			_RefreshTiles();
		}
		else
		{
			_visible_tiles_frozen = true;
		}	
	}
}

void Map::_DrawTiles()
{
	// draw tile borders
	auto& tile_ids = _tile_engine->GetVisibleTiles();
	for (auto& id : tile_ids)
	{
		Tile tile(id);
		_renderer->DrawTile(tile, 0xFFFF77FF);
		if (tile.Contains(_cursor))
		{
			auto pos = tile.GetPosition();
			_renderer->DrawSquare(pos.x, pos.y, 10.0f, 0.f, 0xFF0000FF);
		}
	}
	_tile_engine->PrepareDrawList();
	if (_tile_engine->DrawListCount() > 0)
	{
		_renderer->DrawStaticFeaturesBulk(_tile_engine->DrawListBegin(), _tile_engine->DrawListCount());
	}

}

void Map::RenderScene()
{
	_DrawTiles();
	_renderer->DrawMapBounds();
}