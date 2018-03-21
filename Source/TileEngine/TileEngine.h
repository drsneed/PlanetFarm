#pragma once
#include <map>
#include <set>
#include <blockingconcurrentqueue.h>
#include <Core/StdIncludes.h>
#include <Core/Threadpool.h>
#include "Tile.h"
#include "TileResource.h"
#include "BoundingRect.h"

using namespace moodycamel;

class TileEngine
{
	std::set<TileID> _visible_tile_cache;
	std::map<TileID, std::unique_ptr<TileResource>> _tile_resource_cache;
	Threadpool _threadpool;
	BlockingConcurrentQueue<TileID> _job_queue;
	std::atomic<int> _job_count;
	std::vector<PTP_WORK> _worker_threads;
	
	void _LoadResourcesAsync(std::set<TileID> new_tiles);

public:
	TileEngine();
	~TileEngine();
	std::set<TileID>& Fetch(BoundingRect viewable_area, uint8_t zoom_level);
	bool GetTileContaining(XMFLOAT2 map_point, Tile& tile);
	std::atomic<int>& GetJobCount() { return _job_count; }
	BlockingConcurrentQueue<TileID>& GetJobQueue() { return _job_queue; }
	void WaitForResourceLoaderToFinish();
};