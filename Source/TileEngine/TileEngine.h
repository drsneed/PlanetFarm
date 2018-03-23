#pragma once
#include <map>
#include <set>
#include <unordered_set>
#include <mutex>
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
public:
	struct WorkItem
	{
		TileID tile_id;
		ResourceID resource_id;
	};

private:
	std::set<TileID> _visible_tiles;
	std::map<ResourceID, Resource> _resources;
	std::mutex _resources_mutex;
	std::map<TileID, std::unordered_set<ResourceID>> _tile_resources;
	std::mutex _tile_resources_mutex;
	Threadpool _threadpool;
	BlockingConcurrentQueue<WorkItem> _job_queue;
	std::atomic<int> _job_count;
	std::vector<PTP_WORK> _worker_threads;
	
	void _ExecuteTileLoader(const std::vector<WorkItem>& work);
	const char* const _db_filename;
	bool _InitialLoad(const TileID tile_id);
public:
	TileEngine(const char* const db_filename);
	~TileEngine();
	std::set<TileID>& Fetch(BoundingRect viewable_area, uint8_t zoom_level);
	bool GetTileContaining(XMFLOAT2 map_point, Tile& tile);
	std::atomic<int>& GetJobCount() { return _job_count; }
	BlockingConcurrentQueue<WorkItem>& GetJobQueue() { return _job_queue; }
	void WaitForResourceLoaderToFinish();
	void ProcessTileJob(Db::Connection& conn, const WorkItem& work, const char* thread_name);
	void ProcessResourceJob(Db::Connection& conn, const WorkItem& work, const char* thread_name);
	const char* const GetDatabaseFileName() const { return _db_filename; }

};