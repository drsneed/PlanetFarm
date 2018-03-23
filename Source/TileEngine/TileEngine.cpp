#include "TileEngine.h"
#include <Core/GraphicsWindow.h>
#include <algorithm>
#include <iterator>
#include "DbInterface.h"
#include <sstream>

namespace
{
	const char* thread_names[4] = {
		"Worker 1",
		"Worker 2",
		"Worker 3",
		"Worker 4"
	};
	std::atomic<int> thread_name_id;
	int DatabaseBusyHandler(void* p_connection, int count)
	{
		auto connection = reinterpret_cast<Db::Connection*>(p_connection);
		PRINTF(L"[T%u] DB BUSY %d\n", GetCurrentThreadId(), count);
		return 0;
	}

	void CALLBACK WorkerThread(PTP_CALLBACK_INSTANCE pci, void* data, PTP_WORK)
	{
		auto name = thread_names[thread_name_id.fetch_add(1, std::memory_order_release)];
		PRINTF(L"[%S] READY\n", name);
		auto tile_engine = reinterpret_cast<TileEngine*>(data);
		auto db_connection = Db::Connection(tile_engine->GetDatabaseFileName(), 
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, DatabaseBusyHandler);
		auto& job_queue = tile_engine->GetJobQueue();
		auto& job_count = tile_engine->GetJobCount();
		TileEngine::WorkItem work;
		for(;;)
		{
			// Wait for job
			job_queue.wait_dequeue(work);

			// Check exit condition
			if (work.tile_id == INVALID_TILE_ID)
				break;

			// Process Job
			// If zoom level is max zoom + 1, this is resource id.
			else if (work.resource_id > 0)
				tile_engine->ProcessResourceJob(db_connection, work, name);
			else
				tile_engine->ProcessTileJob(db_connection, work, name);

			// Decrement job count
			job_count.fetch_add(-1, std::memory_order_release);
		}
	}
}


TileEngine::TileEngine(const char* const db_filename)
	: _threadpool(2)
	, _job_count(0)
	, _db_filename(db_filename)
{
	// spawn 4 worker threads
	_worker_threads.push_back(_threadpool.SubmitWork(WorkerThread, this));
	_worker_threads.push_back(_threadpool.SubmitWork(WorkerThread, this));
	_worker_threads.push_back(_threadpool.SubmitWork(WorkerThread, this));
	_worker_threads.push_back(_threadpool.SubmitWork(WorkerThread, this));
}

TileEngine::~TileEngine()
{
	// shutdown the worker threads by sending N invalid tile ids where N = num worker threads
	std::vector<WorkItem> invalid_tiles(_worker_threads.size(), { INVALID_TILE_ID, 0 });
	_job_count.fetch_add(invalid_tiles.size(), std::memory_order::memory_order_release);
	_job_queue.enqueue_bulk(invalid_tiles.begin(), invalid_tiles.size());
	for (auto& worker_thread : _worker_threads)
		_threadpool.Wait(worker_thread, TRUE);
}

void TileEngine::ProcessTileJob(Db::Connection& conn, const WorkItem& work, const char* thread_name)
{
	if (_InitialLoad(work.tile_id))
	{
		std::string tile = Tile(work.tile_id).ToString();
		PRINTF(L"[%S] LOADING TILE %S\n", thread_name, tile.c_str());

		auto resource_ids = DbInterface::QueryResourceIDs(conn, work.tile_id);

		if (resource_ids.size() > 0)
		{
			std::vector<WorkItem> new_work(resource_ids.size());
			size_t i = 0;
			for (auto& resource_id : resource_ids)
			{
				new_work[i++] = WorkItem{ work.tile_id, resource_id };
			}

			_job_count.fetch_add(new_work.size(), std::memory_order::memory_order_release);
			_job_queue.enqueue_bulk(new_work.begin(), new_work.size());

			std::lock_guard<std::mutex> guard(_tile_resources_mutex);
			_tile_resources[work.tile_id].insert(resource_ids.begin(), resource_ids.end());
		}
	}
}

bool TileEngine::_InitialLoad(const TileID tile_id)
{
	bool is_initial_load = false;
	{
		std::lock_guard<std::mutex> guard(_tile_resources_mutex);
		is_initial_load = _tile_resources.count(tile_id) == 0;
		if(is_initial_load)
			_tile_resources[tile_id] = std::unordered_set<ResourceID>();
	}
	return is_initial_load;
}

void TileEngine::ProcessResourceJob(Db::Connection& conn, const WorkItem& work, const char* thread_name)
{
	std::lock_guard<std::mutex> guard(_resources_mutex);
	// if resource does not exist
	if (_resources.count(work.resource_id) == 0)
	{
		PRINTF(L"[%S] PROCESS RESOURCE %u for tile %S\n", thread_name, work.resource_id, Tile(work.tile_id).ToString().c_str());
		auto resource = DbInterface::GetResource(conn, work.resource_id);
		if (resource.id > 0)
		{
			//TODO: Further process resource, then render it.
			switch (resource.type)
			{
			case ResourceType::Empty:
				break;
			case ResourceType::CommericalBuilding1:
			case ResourceType::ResidentialBuilding1:
			case ResourceType::ResidentialBuilding2:
			case ResourceType::IndustrialBuilding1:
				break;
			default:
				break;
			}


			std::string text(resource.payload.blob_size, 0);
			memcpy_s(&text[0], resource.payload.blob_size, resource.payload.blob, resource.payload.blob_size);
			PRINTF(L"[%S] RESOURCE PAYLOAD: %S\n", thread_name, text.c_str());
		}
	}
}

void TileEngine::_ExecuteTileLoader(const std::vector<WorkItem>& work)
{
	_job_count.fetch_add(work.size(), std::memory_order::memory_order_release);
	_job_queue.enqueue_bulk(work.begin(), work.size());
}

void TileEngine::Refresh(BoundingRect viewable_area, uint8_t zoom_level)
{
	XMFLOAT2 top_left, bottom_right;
	viewable_area.GetCorners(top_left, bottom_right);
	int offset = (TILE_SPAN_MAX - TILE_SPAN[zoom_level]) / 2;
	auto left = max(static_cast<int>(floor(top_left.x / TILE_PIXEL_WIDTH)) - offset, 0);
	auto top = min(static_cast<int>(floor(top_left.y / TILE_PIXEL_WIDTH)) - offset, TILE_SPAN[zoom_level] - 1);
	auto right = min(static_cast<int>(floor(bottom_right.x / TILE_PIXEL_WIDTH)) - offset, TILE_SPAN[zoom_level] - 1);
	auto bottom = max(static_cast<int>(floor(bottom_right.y / TILE_PIXEL_WIDTH)) - offset, 0);
	std::set<TileID> visible_tiles;
	for (int x = left; x <= right; ++x)
		for (int y = bottom; y <= top; ++y)
			visible_tiles.insert(Tile(x, y, zoom_level).GetID());

	std::vector<TileID> new_tiles;
	std::set_difference(visible_tiles.begin(), visible_tiles.end(),
		_visible_tiles.begin(), _visible_tiles.end(),
		std::inserter(new_tiles, new_tiles.begin()));

	if (new_tiles.size() > 0)
	{
		std::vector<WorkItem> new_work(new_tiles.size());
		size_t i = 0;
		for (auto& new_tile : new_tiles)
		{
			new_work[i++] = WorkItem{ new_tile, 0 };
		}
		_ExecuteTileLoader(new_work);
	}

	_visible_tiles = visible_tiles;
}

bool TileEngine::GetTileContaining(XMFLOAT2 map_point, Tile& tile)
{
	int level_0_span = pow(2, TILE_MAX_ZOOM);
	int this_level_span = pow(2, tile.z);
	int offset = (level_0_span - this_level_span) / 2;

	auto x = static_cast<int>(floor(map_point.x / TILE_PIXEL_WIDTH)) - offset;
	auto y = static_cast<int>(floor(map_point.y / TILE_PIXEL_WIDTH)) - offset;

	if (x >= 0 && x < this_level_span && y >= 0 && y < this_level_span)
	{
		tile.x = static_cast<uint16_t>(x);
		tile.y = static_cast<uint16_t>(y);
		return true;
	}

	return false;
}


void TileEngine::WaitForResourceLoaderToFinish()
{
	while (_job_count.load(std::memory_order_acquire) != 0)
		continue;
}

void TileEngine::CollectVisibleThings()
{
	// TODO: collect all visible resources from parent and children tiles...
	//       within visibility range of item type (i.e. buildings visibile range could 12 - 14)
	//		 for rendering, build sorted array based on item's type

	for (auto& visible_tile : _visible_tiles)
	{
		_draw_queue.enqueue(Thing{ visible_tile });
	}
}

bool TileEngine::Collect(Thing& thing)
{
	return _draw_queue.try_dequeue(thing);
}