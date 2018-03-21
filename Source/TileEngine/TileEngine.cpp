#include "TileEngine.h"
#include <Core/GraphicsWindow.h>
#include <algorithm>
#include <iterator>
#include "DbInterface.h"
#include <sstream>
#include <bitset>

// The ZOOM_MASK itself is an invalid tile (0, 0, 15)
// so we use that value to signal the worker threads to exit
#define EXIT_SIGNAL ZOOM_MASK
//uint32_t zoom_bits = job_id & ZOOM_MASK;
//std::bitset<32> out(zoom_bits);
//std::stringstream ss;
//ss << out;
//PRINTF(L"zoom_bits: %S\n", out.to_string().c_str());
//ss.clear();
//out = TILE_MAX_ZOOM + 1;
//ss << out;
//PRINTF(L"zoom 15: %S\n", out.to_string().c_str());
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
		uint32_t job_id;
		for(;;)
		{
			// Wait for job
			job_queue.wait_dequeue(job_id);

			// Check exit condition
			if (job_id == EXIT_SIGNAL)
				break;

			// Process Job
			// If zoom level is max zoom + 1, this is resource id.
			else if ((job_id & ZOOM_MASK) == (TILE_MAX_ZOOM + 1))
				tile_engine->ProcessResourceJob(db_connection, job_id);
			else
				tile_engine->ProcessTileJob(db_connection, job_id);

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
	std::vector<TileID> invalid_tiles(_worker_threads.size(), EXIT_SIGNAL);
	_job_count.fetch_add(invalid_tiles.size(), std::memory_order::memory_order_release);
	_job_queue.enqueue_bulk(invalid_tiles.begin(), invalid_tiles.size());
	for (auto& worker_thread : _worker_threads)
		_threadpool.Wait(worker_thread, TRUE);
}

void TileEngine::ProcessTileJob(Db::Connection& conn, TileID tile_id)
{
	//PRINTF(L"[T%u] PROCESS TILE %S\n", GetCurrentThreadId(), Tile(tile_id).GetQuadKey().c_str());
	auto resource_ids = DbInterface::QueryResourceIDs(conn, tile_id);
	if (resource_ids.size() > 0)
	{
		_job_count.fetch_add(resource_ids.size(), std::memory_order::memory_order_release);
		_job_queue.enqueue_bulk(resource_ids.begin(), resource_ids.size());
	}
}

void TileEngine::ProcessResourceJob(Db::Connection& conn, ResourceID resource_id)
{
	//auto resource = DbInterface::LoadResource(conn, resource_id);
	PRINTF(L"[T%u] PROCESS RESOURCE %u\n", GetCurrentThreadId(), resource_id);
}

void TileEngine::_LoadResourcesAsync(std::set<TileID> new_tiles)
{
	_job_count.fetch_add(new_tiles.size(), std::memory_order::memory_order_release);
	_job_queue.enqueue_bulk(new_tiles.begin(), new_tiles.size());
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

	std::set<TileID> new_tiles;
	std::set_difference(visible_tiles.begin(), visible_tiles.end(),
		_visible_tiles.begin(), _visible_tiles.end(),
		std::inserter(new_tiles, new_tiles.begin()));

	_LoadResourcesAsync(new_tiles);

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