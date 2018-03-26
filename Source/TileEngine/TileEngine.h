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
#include "Models/Feature.h"
#include "Models/BoundingRect.h"
using namespace moodycamel;



class TileEngine
{
public:
	struct WorkItem
	{
		TileID tile_id;
		FeatureID feature_id;
	};

private:
	std::set<TileID> _visible_tiles;
	std::map<FeatureID, Feature> _features;
	std::mutex _features_mutex;
	std::map<TileID, std::unordered_set<FeatureID>> _tile_features;
	std::mutex _tile_features_mutex;
	Threadpool _threadpool;
	BlockingConcurrentQueue<WorkItem> _job_queue;
	std::atomic<int> _job_count;
	std::vector<PTP_WORK> _worker_threads;
	std::vector<StaticFeature> _draw_list;
	void _ExecuteTileLoader(const std::vector<WorkItem>& work);
	const char* const _db_filename;
	bool _InitialLoad(const TileID tile_id, const char* thread_name);
	size_t _draw_list_index;
	bool _build_draw_list;
	void _BuildDrawList();
public:
	TileEngine(const char* const db_filename);
	~TileEngine();
	void Refresh(BoundingRect viewable_area, uint8_t zoom_level);
	bool GetTileContaining(XMFLOAT2 map_point, Tile& tile);
	std::atomic<int>& GetJobCount() { return _job_count; }
	BlockingConcurrentQueue<WorkItem>& GetJobQueue() { return _job_queue; }
	void WaitForBusyThreads();
	void ProcessTileJob(Db::Connection& conn, const WorkItem& work, const char* thread_name);
	void ProcessFeatureJob(Db::Connection& conn, const WorkItem& work, const char* thread_name);
	const char* const GetDatabaseFileName() const { return _db_filename; }
	const std::set<TileID>& GetVisibleTiles() { return _visible_tiles; }

	void PrepareDrawList();
	StaticFeature* DrawListBegin() { ASSERT(_draw_list.size() > 0); return &_draw_list[0]; }
	size_t DrawListCount() { return _draw_list.size(); }

};