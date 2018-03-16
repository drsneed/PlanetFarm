#include "Map.h"
#include <cmath>
#include <TileEngine/Tile.h>

Map::Map(std::shared_ptr<Camera> camera)
	: _tile_engine(std::make_unique<TileEngine>(camera))
	, _renderer(std::make_unique<MapRenderer>())
	, _zoom(0, 0)
	, _center((pow(2, TILE_MAX_ZOOM) * TILE_PIXEL_WIDTH) / 2.0f)
	, _cam(camera)
{

	_cam->NotifyPosChange(std::bind(&Map::HandleCameraPosChangedEvent, this, std::placeholders::_1));
}

void Map::HandleCameraPosChangedEvent(XMFLOAT3 position)
{
	
}

void Map::ZoomIn()
{
	_zoom.inc();
	char buf[8];
	sprintf_s(buf, "%s\n", _zoom.ToString().c_str());
	OutputDebugStringA(buf);
}

void Map::ZoomOut()
{
	_zoom.dec();
	char buf[8];
	sprintf_s(buf, "%s\n", _zoom.ToString().c_str());
	OutputDebugStringA(buf);
}

void Map::ZoomTo(const ZoomLevel& level)
{
	_zoom = level;
}

auto Map::GetZoom() const -> ZoomLevel
{
	return _zoom;
}

XMFLOAT3 Map::GetMouseCursorPosition()
{
	XMFLOAT3 origin, direction;
	_cam->ComputeRayFromMouseCursor(origin, direction);
	if (isnan(direction.x) || isnan(direction.y) || isnan(direction.z))
	{
		return XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	XMVECTOR o = XMLoadFloat3(&origin);
	XMVECTOR d = XMLoadFloat3(&direction);

	auto bbox = BoundingBox
	{ 
		{_center, 0.0f, _center}, // Center of bbox
		{_center, 0.0f, _center} // Extents of bbox from center
	};

	float distance;
	if (bbox.Intersects(o, d, distance))
	{
		auto intersection_point = o + d * distance;
		XMFLOAT3 result;
		XMStoreFloat3(&result, intersection_point);
		return result;
	}
	else
	{
		return XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
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
}

void Map::RenderScene()
{
	for (auto& tile : _visible_tiles)
	{
		_renderer->DrawTile(tile);
	}
	_renderer->DrawMapBounds();
}