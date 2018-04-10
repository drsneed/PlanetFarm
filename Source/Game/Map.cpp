#include "Map.h"
#include <cmath>
#include <TileEngine/Tile.h>
#include <time.h>

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
	, _generator(171)
	, _scale_test(1)
	, _coastlines(_generator.GetCoastlines2())
{
	_cam->NotifyPosChange(std::bind(&Map::HandleCameraPosChangedEvent, this, std::placeholders::_1));
	_cam->UpdateGpuBuffer();
	GetCenterScreen(true);
	_RefreshTiles();

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

auto Map::ZoomInPoint(const MapPoint& point) -> MapPoint
{
	MapPoint dist_to_center = { point.x - MAP_ABSOLUTE_CENTER, point.y - MAP_ABSOLUTE_CENTER };
	dist_to_center.x *= 2.0f;
	dist_to_center.y *= 2.0f;
	return XMFLOAT2{ MAP_ABSOLUTE_CENTER + dist_to_center.x, MAP_ABSOLUTE_CENTER + dist_to_center.y };
}

auto Map::ZoomOutPoint(const MapPoint& point) -> MapPoint
{
	MapPoint dist_to_center = { point.x - MAP_ABSOLUTE_CENTER, point.y - MAP_ABSOLUTE_CENTER };
	dist_to_center.x /= 2.0f;
	dist_to_center.y /= 2.0f;
	return XMFLOAT2{ MAP_ABSOLUTE_CENTER + dist_to_center.x, MAP_ABSOLUTE_CENTER + dist_to_center.y };
}

void Map::ZoomIn()
{
	if (_zoom.inc())
	{
		MapPoint adjusted_cursor = ZoomInPoint(_cursor);
		auto cam_pos = _cam->GetPosition();
		auto diff = XMFLOAT2{ _cursor.x - cam_pos.x, _cursor.y - cam_pos.z };
		_cam->SetPosition(adjusted_cursor.x - diff.x, cam_pos.y, adjusted_cursor.y - diff.y, true);
		GetCursor(true);
	}
}

void Map::ZoomOut()
{
	if (_zoom.dec())
	{
		MapPoint adjusted_cursor = ZoomOutPoint(_cursor);
		auto cam_pos = _cam->GetPosition();
		auto diff = XMFLOAT2{ _cursor.x - cam_pos.x, _cursor.y - cam_pos.z };
		_cam->SetPosition(adjusted_cursor.x - diff.x, cam_pos.y, adjusted_cursor.y - diff.y, true);
		GetCursor(true);
	}
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
	else if (event.type == EventType::MouseClick)
	{
		auto tile = _tile_engine->GetTileContaining(_cursor, _zoom.major_part);
		if (tile.IsValid())
		{
			auto tile_pos = tile.GetPosition();
			auto diff = XMFLOAT2(tile_pos.x - _cursor.x, tile_pos.y - _cursor.y);
			PRINTF(L"(%.2f, %.2f)\n", -diff.x, -diff.y);
		}

	}
	if (event.code == GraphicsWindow::Event::Code::Up && event.type == GraphicsWindow::Event::Type::KeyRelease)
	{
		_scale_test += 2;
		if (_scale_test > 16)
			_scale_test = 16;
	}
	if (event.code == GraphicsWindow::Event::Code::Down && event.type == GraphicsWindow::Event::Type::KeyRelease)
	{
		_scale_test -= 2;
		if (_scale_test < 1)
			_scale_test = 1;
	}

	if (event.code == GraphicsWindow::Event::Code::F)
	{
		if (event.type == GraphicsWindow::Event::Type::KeyRelease)
			_visible_tiles_frozen = !_visible_tiles_frozen;
	}
	else if (event.code == GraphicsWindow::Event::Code::Home)
	{
		_generator = LandGenerator(time(NULL));
		_coastlines = _generator.GetCoastlines2();
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
		//if (tile.Contains(_cursor))
		//{
		//	auto pos = tile.GetPosition();
		//	_renderer->DrawSquare(pos.x, pos.y, 10.0f, 0.f, 0xFF0000FF);
		//}
	}
	_tile_engine->PrepareDrawLists();
	if (_tile_engine->DynamicFeatureDrawListCount() > 0)
	{
		_renderer->DrawDynamicFeaturesBulk(_tile_engine->DynamicFeaturesDrawListBegin(), _tile_engine->DynamicFeatureDrawListCount());
	}
	if (_tile_engine->StaticFeatureDrawListCount() > 0)
	{
		_renderer->DrawStaticFeaturesBulk(_tile_engine->StaticFeaturesDrawListBegin(), _tile_engine->StaticFeatureDrawListCount());
	}

}

void Map::RenderScene()
{
	auto& mesh = _generator.GetMesh();
	auto ghost_index_tris = mesh.GetGhostIndexTris();
	unsigned water = 0x0094FFFF;
	unsigned land = 0x707C4FFF;
	unsigned coast = 0xEEEEEEFF;
	//for (int i = 0; i < ghost_index_tris; i += 3)
	//{
	//	_renderer->DrawLine(mesh.vertices[mesh.triangles[i]].Shrink(), mesh.vertices[mesh.triangles[i+1]].Shrink(), color);
	//	_renderer->DrawLine(mesh.vertices[mesh.triangles[i+1]].Shrink(), mesh.vertices[mesh.triangles[i+2]].Shrink(), color);
	//	_renderer->DrawLine(mesh.vertices[mesh.triangles[i+2]].Shrink(), mesh.vertices[mesh.triangles[i]].Shrink(), color);
	//}

	//for (int s = 0; s < ghost_index_tris; s++) 
	//{
	//	auto r = mesh.triangles[s];
	//	auto t1 = s_to_t(s);
	//	auto t2 = s_to_t(mesh.half_edges[s]);
	//	auto v1 = mesh.vertices[r].Shrink();
	//	auto v2 = mesh.region_vertices[t1].Shrink();
	//	auto v3 = mesh.region_vertices[t2].Shrink();
	//	_renderer->DrawTriangle(v1, v2, v3, color);
	//}

	auto region_count = mesh.GetRegionCount();
	auto ghost_index_verts = mesh.GetGhostIndexVerts();



	for (int i = 0; i < _coastlines.size(); ++i)
	{
		for (int j = 0; j < _coastlines[i].size(); j+= _scale_test)
		{
			int j1 = j;
			int j2 = j1 + _scale_test;
			if (j2 >= _coastlines[i].size()) j2 = 0;
			_renderer->DrawLine(mesh.region_vertices[_coastlines[i][j1]].Shrink(), mesh.region_vertices[_coastlines[i][j2]].Shrink(), 0xFF0000FF);
		}
	}

	
	if(false)
	{
		int r1, r2;
		for (int i = 0; i < ghost_index_verts; ++i)
		{
			if (!_generator.IsCoast(i))
				continue;
			unsigned color = coast;//_generator.IsCoast(i) ? coast : land;

			std::vector<int> vertices = mesh.GetRegionVerticesI(i);
			auto center = mesh.vertices[i].Shrink();
			for (int v = 0; v < vertices.size(); ++v)
			{
				auto v2 = v + 1;
				if (v2 == vertices.size()) v2 = 0;
				mesh.GetFlankingRegions(vertices[v], vertices[v2], r1, r2);
				if (r1 >= 0 && r2 >= 0)
				{
					bool coastal = (_generator.IsWater(r1) && !_generator.IsWater(r2)) || (_generator.IsWater(r2) && !_generator.IsWater(r1));
					if (coastal && _generator.IsCoastlineVertex(vertices[v]) && _generator.IsCoastlineVertex(vertices[v2]))
						_renderer->DrawLine(mesh.region_vertices[vertices[v]].Shrink(), mesh.region_vertices[vertices[v2]].Shrink(), 0x2255FFFF);
				}
			}
		}
	}
	



	//std::vector<XMFLOAT2> coastline_verts(coastlines.size());
	//std::vector<int> coastlines = _generator.GetCoastlines();

	//for (int p = 0; p < coastlines.size(); ++p)
	//{
	//	//_renderer->DrawSquare(mesh.region_vertices[coastlines[p]].x,
	//		//mesh.region_vertices[coastlines[p]].y, 0.25f, 0.0f, 0x00FF00FF);
	//	//coastline_verts[p] = mesh.region_vertices[coastlines[p]].Shrink();
	//	
	//}
	//_renderer->DrawPoints(coastline_verts, 0xFF0000FF);


	//_renderer->DrawPoints(_temp_land.vertices, 0xFFFFFFFF);
	_renderer->DrawSquare(10.f, 10.f, 20.f, 0.f, 0x00FF00FF);
	//_DrawTiles();
	//_renderer->DrawMapBounds();
}

//auto edges = mesh.GetRegionEdges(i);
//for (int e = 0; e < edges.size(); ++e)
//{
//	auto v0 = mesh.region_vertices[s_to_t(e)];

//	auto noisy_edges = _generator.GetNoisyEdges(e);
//	//auto center = mesh.vertices[i].Shrink();
//	if (noisy_edges.size() > 0)
//	{
//		_renderer->DrawLine(v0.Shrink(), noisy_edges[0].Shrink(), 0x2255FFFF);
//		for (int v = 0; v < noisy_edges.size() - 1; ++v)
//		{

//			//_renderer->DrawTriangle(noisy_edges[v2].Shrink(), noisy_edges[v].Shrink(), center, color);

//			_renderer->DrawLine(noisy_edges[v].Shrink(), noisy_edges[v+1].Shrink(), 0x2255FFFF);
//		}
//	}
//	else
//	{
//		auto e2 = e + 1;
//		if (e2 >= edges.size())
//		{
//			e2 = 0;
//		}
//		_renderer->DrawLine(v0.Shrink(), mesh.region_vertices[s_to_t(e)].Shrink(), 0x2255FFFF);
//	}
//}


/*
for (int r = 0; r < region_count; r++)
{
auto edges = mesh.GetRegionEdges(r);

int last_t = mesh.s_inner_t(out_s[0]);
ctx.fillStyle = ctx.strokeStyle = colormap.biome(map, r);
ctx.beginPath();
ctx.moveTo(mesh.t_x(last_t), mesh.t_y(last_t));
for (let s of out_s) {
if (!noisyEdge || !colormap.side(map, s).noisy) {
let first_t = mesh.s_outer_t(s);
ctx.lineTo(mesh.t_x(first_t), mesh.t_y(first_t));
}
else {
for (let p of map.s_lines[s]) {
ctx.lineTo(p[0], p[1]);
}
}
}
ctx.fill();
}*/