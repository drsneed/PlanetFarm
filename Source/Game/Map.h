#pragma once
#include <Core/StdIncludes.h>
#include <Core/GraphicsWindow.h>
#include <TileEngine/TileEngine.h>
#include "MapRenderer.h"
#include "Camera.h"

typedef XMFLOAT2 MapPoint;

class Map
{
public:
	struct ZoomLevel
	{
		uint8_t major_part;
		uint8_t minor_part;

		ZoomLevel(uint8_t major, uint8_t minor)
			: major_part(major)
			, minor_part(minor)
		{
			// Bounds check major [0 .. 15]
			ASSERT(major_part <= TILE_MAX_ZOOM);

			// Bounds check minor [0 .. 9]
			ASSERT(minor_part < 10);
		}

		void inc()
		{
			if (minor_part == 9)
			{
				if (major_part  < TILE_MAX_ZOOM)
				{
					major_part++;
					minor_part = 0;
				}
			}
			else
			{
				minor_part++;
			}
		}

		void dec()
		{
			if (minor_part == 0)
			{
				if (major_part > 0)
				{
					major_part--;
					minor_part = 9;
				}
			}
			else
			{
				minor_part--;
			}
		}

		std::string ToString()
		{
			return std::to_string(major_part) + "." + std::to_string(minor_part);
		}
	};
private:
	ZoomLevel _zoom;
	
	MapPoint _cursor;
	MapPoint _cam_center;
	std::unique_ptr<TileEngine> _tile_engine;
	std::shared_ptr<Camera> _cam;
	std::unique_ptr<MapRenderer> _renderer;
	std::vector<Tile> _visible_tiles;
	BoundingRect _CalculateVisibleArea();
	void _UpdateVisibleTiles();
public:
	Map(std::shared_ptr<Camera> camera);
	void RenderScene();
	void ZoomIn();
	void ZoomOut();
	void ZoomTo(const ZoomLevel& level);
	auto GetZoom() const -> ZoomLevel;
	/// Returns cursor position in world space
	MapPoint GetCursor(bool refresh = false);
	MapPoint GetCamCenter(bool refresh = false);
	void HandleCameraPosChangedEvent(XMFLOAT3 position);
	void Tick(float delta_time);
	void HandleEvent(const GraphicsWindow::Event& event);
};