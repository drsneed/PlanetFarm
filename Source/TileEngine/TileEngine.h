#pragma once
#include <map>
#include <set>
#include <blockingconcurrentqueue.h>
#include <Core/StdIncludes.h>
#include <Core/Threadpool.h>
#include <Core/Db.h>
#include "Tile.h"
#include "Resource.h"
#include "BoundingRect.h"

using namespace moodycamel;

class TileEngine
{
	std::set<TileID> _visible_tiles;
	std::map<TileID, std::vector<ResourceID>> _tile_resources;
	Threadpool _threadpool;
	BlockingConcurrentQueue<TileID> _job_queue;
	std::atomic<int> _job_count;
	std::vector<PTP_WORK> _worker_threads;
	
	void _LoadResourcesAsync(std::set<TileID> new_tiles);
	const char* const _db_filename;


public:
	TileEngine(const char* const db_filename);
	~TileEngine();
	std::set<TileID>& Fetch(BoundingRect viewable_area, uint8_t zoom_level);
	bool GetTileContaining(XMFLOAT2 map_point, Tile& tile);
	std::atomic<int>& GetJobCount() { return _job_count; }
	BlockingConcurrentQueue<TileID>& GetJobQueue() { return _job_queue; }
	void WaitForResourceLoaderToFinish();
	void ProcessTileJob(Db::Connection& conn, TileID tile_id);
	void ProcessResourceJob(Db::Connection& conn, ResourceID resource_id);
	const char* const GetDatabaseFileName() const { return _db_filename; }
};