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
#include "ModelsManager.h"
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
	BoundingRect _visible_area;
	std::set<TileID> _visible_tiles;
	std::map<FeatureID, Feature> _features;
	std::mutex _features_mutex;
	std::map<TileID, std::unordered_set<FeatureID>> _tile_features;
	std::mutex _tile_features_mutex;
	Threadpool _threadpool;
	BlockingConcurrentQueue<WorkItem> _job_queue;
	ModelsManager _models_manager;
	std::atomic<int> _job_count;
	std::vector<PTP_WORK> _worker_threads;
	std::vector<StaticFeature> _static_feature_draw_list;
	std::vector<DynamicFeature*> _dynamic_feature_draw_list;
	uint8_t _zoom;
	void _ExecuteTileLoader(const std::vector<WorkItem>& work);
	const char* const _db_filename;
	bool _InitialLoad(const TileID tile_id, const char* thread_name);
	bool _build_draw_lists;
	void _BuildDrawLists();
	Tile _ContainsRecursive(Tile tile, const XMFLOAT2& top_left, const XMFLOAT2& bottom_right);
	void _CollectVisibleFeaturesRecursive(Tile tile, const XMFLOAT2& top_left, const XMFLOAT2& bottom_right);
	Tile _GetChildTileContaining(Tile tile, const XMFLOAT2& top_left, const XMFLOAT2& bottom_right);
public:
	TileEngine(const char* const db_filename);
	~TileEngine();
	void Refresh(BoundingRect visible_area, uint8_t zoom_level);
	Tile GetTileContaining(XMFLOAT2 map_point, uint8_t zoom_level);
	Tile GetTileContaining(BoundingRect visible_area);

	std::atomic<int>& GetJobCount() { return _job_count; }
	BlockingConcurrentQueue<WorkItem>& GetJobQueue() { return _job_queue; }
	void WaitForBusyThreads();
	void ProcessTileJob(Db::Connection& conn, const WorkItem& work, const char* thread_name);
	void ProcessFeatureJob(Db::Connection& conn, const WorkItem& work, const char* thread_name);
	const char* const GetDatabaseFileName() const { return _db_filename; }
	const std::set<TileID>& GetVisibleTiles() { return _visible_tiles; }

	void PrepareDrawLists();
	StaticFeature* StaticFeaturesDrawListBegin() { ASSERT(_static_feature_draw_list.size() > 0); return &_static_feature_draw_list[0]; }
	size_t StaticFeatureDrawListCount() { return _static_feature_draw_list.size(); }
	DynamicFeature** DynamicFeaturesDrawListBegin() { ASSERT(_dynamic_feature_draw_list.size() > 0); return &_dynamic_feature_draw_list[0]; }
	size_t DynamicFeatureDrawListCount() { return _dynamic_feature_draw_list.size(); }

};