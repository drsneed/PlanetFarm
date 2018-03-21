#include "TileEngine.h"
#include <Core/GraphicsWindow.h>
#include <algorithm>
#include <iterator>

namespace
{
	void CALLBACK ResourceLoader(PTP_CALLBACK_INSTANCE pci, void* data, PTP_WORK)
	{
		auto tile_engine = reinterpret_cast<TileEngine*>(data);
		auto& job_queue = tile_engine->GetJobQueue();
		auto& job_count = tile_engine->GetJobCount();
		TileID tile_id;
		for (;;)
		{
			job_queue.wait_dequeue(tile_id);

			// Process job
			PRINTF(L"[T%d] LOAD RESOURCE %d\n", GetCurrentThreadId(), tile_id);

			job_count.fetch_add(-1, std::memory_order_release);
		}

	}
}


TileEngine::TileEngine()
	: _threadpool(2)
	, _job_count(0)
{
	// spawn 4 worker threads
	_worker_threads.push_back(_threadpool.SubmitWork(ResourceLoader, this));
	_worker_threads.push_back(_threadpool.SubmitWork(ResourceLoader, this));
	_worker_threads.push_back(_threadpool.SubmitWork(ResourceLoader, this));
	_worker_threads.push_back(_threadpool.SubmitWork(ResourceLoader, this));
}

TileEngine::~TileEngine()
{
	//for (auto& worker_thread : _worker_threads)
	//{
	//	_threadpool.Wait(worker_thread, TRUE);
	//}
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
		_visible_tile_cache.begin(), _visible_tile_cache.end(),
		std::inserter(new_tiles, new_tiles.begin()));

	_LoadResourcesAsync(new_tiles);

	_visible_tile_cache = visible_tiles;
	return _visible_tile_cache;
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