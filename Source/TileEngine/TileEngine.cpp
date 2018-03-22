#include "TileEngine.h"
#include <Core/GraphicsWindow.h>
#include <algorithm>
#include <iterator>
#include "DbInterface.h"


// The ZOOM_MASK itself is an invalid tile (0, 0, 15)
// so we use that value to signal the worker threads to exit
#define EXIT_SIGNAL ZOOM_MASK
namespace
{
	int DatabaseBusyHandler(void* p_connection, int count)
	{
		auto connection = reinterpret_cast<Db::Connection*>(p_connection);
		PRINTF(L"[T%u] DB BUSY %d\n", GetCurrentThreadId(), count);
		return 0;
	}

	void CALLBACK WorkerThread(PTP_CALLBACK_INSTANCE pci, void* data, PTP_WORK)
	{
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
			if (work.tile_id == EXIT_SIGNAL)
				break;

			// Process Job
			// If zoom level is max zoom + 1, this is resource id.
			else if (work.resource_id > 0)
				tile_engine->ProcessResourceJob(db_connection, work);
			else
				tile_engine->ProcessTileJob(db_connection, work);

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
	// shutdown the worker threads by sending N exit signals where N = num worker threads
	std::vector<WorkItem> invalid_tiles(_worker_threads.size(), { EXIT_SIGNAL, 0 });
	_job_count.fetch_add(invalid_tiles.size(), std::memory_order::memory_order_release);
	_job_queue.enqueue_bulk(invalid_tiles.begin(), invalid_tiles.size());
	for (auto& worker_thread : _worker_threads)
		_threadpool.Wait(worker_thread, TRUE);
}

void TileEngine::ProcessTileJob(Db::Connection& conn, const WorkItem& work)
{
	//PRINTF(L"[T%u] PROCESS TILE %S\n", GetCurrentThreadId(), Tile(tile_id).GetQuadKey().c_str());
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
	}

	std::lock_guard<std::mutex> guard(_tile_resource_map_mutex);

	// if tile does not exist
	if (_tile_resource_map.count(work.tile_id) == 0)
	{
		_tile_resource_map[work.tile_id] = { work.resource_id };
	}
	else
	{
		_tile_resource_map[work.tile_id].insert(work.resource_id);
	}
	
}

void TileEngine::ProcessResourceJob(Db::Connection& conn, const WorkItem& work)
{
	std::lock_guard<std::mutex> guard(_resources_mutex);

	// if resource does not exist
	if (_resources.count(work.resource_id) == 0)
	{
		PRINTF(L"[T%u] PROCESS RESOURCE %u\n", GetCurrentThreadId(), work.resource_id);
		Resource resource;
		if (DbInterface::GetResource(conn, work.tile_id, work.resource_id, resource))
		{
			std::string text(resource.payload.blob_size, 0);
			memcpy_s(&text[0], resource.payload.blob_size, resource.payload.blob, resource.payload.blob_size);
			PRINTF(L"[T%u] LOADED RESOURCE %S\n", GetCurrentThreadId(), text.c_str());
			_resources[work.resource_id] = std::move(resource);
		}
	}
}

void TileEngine::_ExecuteTileLoader(const std::vector<WorkItem>& work)
{
	_job_count.fetch_add(work.size(), std::memory_order::memory_order_release);
	_job_queue.enqueue_bulk(work.begin(), work.size());
}

std::set<TileID>& TileEngine::Fetch(BoundingRect viewable_area, uint8_t zoom_level)
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
	return _visible_tiles;
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