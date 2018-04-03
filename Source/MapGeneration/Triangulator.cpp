#include "Triangulator.h"
#include <Core/DebugTools.h>
#include <cmath>
#include <algorithm>
//edges.erase(std::remove_if(edges.begin(), edges.end(),
//	[](int& e) { return e == -1; }), edges.end());


namespace 
{
	double dist(const WidePoint& a, const WidePoint& b)
	{
		auto dx = a.x - b.x;
		auto dy = a.y - b.y;
		auto result = dx * dx + dy * dy;
		return result;
	}

	float area(const WidePoint& p, const WidePoint& q, const WidePoint& r)
	{
		return (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
	}

	float circumradius(const WidePoint& a, const WidePoint& b, const WidePoint& c)
	{
		auto bx = b.x - a.x;
		auto by = b.y - a.y;
		auto cx = c.x - a.x;
		auto cy = c.y - a.y;

		auto bl = bx * bx + by * by;
		auto cl = cx * cx + cy * cy;

		if (bl == 0.0 || cl == 0.0) return  std::numeric_limits<double>::infinity();

		auto d = bx * cy - by * cx;
		if (d == 0.0) return  std::numeric_limits<double>::infinity();

		auto x = (cy * bl - by * cl) * 0.5 / d;
		auto y = (bx * cl - cx * bl) * 0.5 / d;

		return x * x + y * y;
	}

	WidePoint circumcenter(const WidePoint& a, const WidePoint& b, const WidePoint& c)
	{
		auto bx = b.x - a.x;
		auto by = b.y - a.y;
		auto cx = c.x - a.x;
		auto cy = c.y - a.y;

		auto bl = bx * bx + by * by;
		auto cl = cx * cx + cy * cy;

		auto d = bx * cy - by * cx;

		auto x = (cy * bl - by * cl) * 0.5 / d;
		auto y = (bx * cl - cx * bl) * 0.5 / d;

		return {
		  a.x + x,
		  a.y + y
		};
	}

	// I'm skeptical about this converted routine
	double compare(std::vector<WidePoint>& coords, uint32_t i, uint32_t j, const WidePoint& center)
	{
		auto d1 = dist(coords[i], center);
		auto d2 = dist(coords[j], center);
		double result = d1 - d2;
		if (!isnan(result) && result != 0.0)
			return result;
		result = (coords[i].x - coords[j].x);
		if (!isnan(result) && result != 0.0)
			return result;
		result = (coords[i].y - coords[j].y);
		return result;
	}

	void swap(std::vector<uint32_t>& ids, int i, int j) 
	{
		auto tmp = ids[i];
		ids[i] = ids[j];
		ids[j] = tmp;
	}

	void quicksort(std::vector<uint32_t>& ids, std::vector<WidePoint>& coords, int left, int right, const WidePoint& center)
	{
		int i, j, temp;

		if (right - left <= 20) 
		{
			for (i = left + 1; i <= right; i++) 
			{
				temp = ids[i];
				j = i - 1;
				while (j >= left && compare(coords, ids[j], temp, center) > 0)
				{
					//ids[j + 1] = ids[j--];
					ids[j + 1] = ids[j];
					j--;
				}
				ids[j + 1] = temp;
			}
		}
		else 
		{
			auto median = (left + right) >> 1;
			i = left + 1;
			j = right;
			swap(ids, median, i);
			if (compare(coords, ids[left], ids[right], center) > 0) swap(ids, left, right);
			if (compare(coords, ids[i], ids[right], center) > 0) swap(ids, i, right);
			if (compare(coords, ids[left], ids[i], center) > 0) swap(ids, left, i);

			temp = ids[i];
			while (true) 
			{
				do i++; while (compare(coords, ids[i], temp, center) < 0);
				do j--; while (compare(coords, ids[j], temp, center) > 0);
				if (j < i) break;
				swap(ids, i, j);
			}
			ids[left + 1] = ids[j];
			ids[j] = temp;

			if (right - i + 1 >= j - left) 
			{
				quicksort(ids, coords, i, right, center);
				quicksort(ids, coords, left, j - 1, center);
			}
			else 
			{
				quicksort(ids, coords, left, j - 1, center);
				quicksort(ids, coords, i, right, center);
			}
		}
	}

	struct HullNode
	{
		int i;
		WidePoint p;
		int t;
		bool removed;
		HullNode* prev;
		HullNode* next;
	};

	// create a new node in a doubly linked list
	HullNode* InsertNode(HullNode* node, std::vector<WidePoint> verts, int i, HullNode* prev)
	{
		node->i = i;
		node->p = verts[i];
		node->t = 0;
		node->prev = nullptr;
		node->next = nullptr;
		node->removed = false;

		if (prev) 
		{
			node->next = prev->next;
			node->prev = prev;
			prev->next->prev = node;
			prev->next = node;
		}
		return node;
	}

	HullNode* RemoveNode(HullNode* node) 
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;
		node->removed = true;
		return node->prev;
	}
}


Triangulator::Triangulator(const std::vector<WidePoint>& verts)
	: _ids(verts.size())
	, _verts(verts)

{
	//for (size_t ii = 0; ii < verts.size(); ++ii)
	//{
	//	_verts[ii] = { static_cast<double>(verts[ii].x), 
	//		static_cast<double>(verts[ii].y) };
	//}
	uint32_t n = static_cast<uint32_t>(_verts.size());

	auto min_x = std::numeric_limits<double>::infinity();
	auto min_y = std::numeric_limits<double>::infinity();
	auto max_x = -std::numeric_limits<double>::infinity();
	auto max_y = -std::numeric_limits<double>::infinity();

	for (size_t i = 0; i < n; i++) 
	{
		auto x = _verts[i].x;
		auto y = _verts[i].y;
		if (x < min_x) min_x = x;
		if (y < min_y) min_y = y;
		if (x > max_x) max_x = x;
		if (y > max_y) max_y = y;
		_ids[i] = i;
	}
	
	WidePoint c { (min_x + max_x) / 2.0, (min_y + max_y) / 2.0 };

	auto min_dist = std::numeric_limits<double>::infinity();
	uint32_t i0, i1, i2;

	// pick a seed point close to the centroid
	uint32_t i;
	for (i = 0; i < n; i++) 
	{
		auto d = dist(c, _verts[i]);
		if (d < min_dist) 
		{
			i0 = i;
			min_dist = d;
		}
	}

	min_dist = std::numeric_limits<double>::infinity();

	// find the point closest to the seed
	for (i = 0; i < n; i++) 
	{
		if (i == i0) continue;
		auto d = dist(_verts[i0] , _verts[i]);
		if (d < min_dist && d > 0.0) 
		{
			i1 = i;
			min_dist = d;
		}
	}

	float min_radius = std::numeric_limits<float>::infinity();

	// find the third point which forms the smallest circumcircle with the first two
	for (i = 0; i < n; i++) 
	{
		if (i == i0 || i == i1) continue;

		float r = circumradius(_verts[i0], _verts[i1], _verts[i]);

		if (r < min_radius) 
		{
			i2 = i;
			min_radius = r;
		}
	}

	// throw error. no delaunay triangulation exists for this set of vertices
	ASSERT(min_radius != std::numeric_limits<double>::infinity());

	// swap the order of the seed points for counter-clockwise orientation
	if (area(_verts[i0], _verts[i1], _verts[i2]) < 0.0)
	{
		auto tmp = i1;
		i1 = i2;
		i2 = tmp;
	}

	auto v0 = _verts[i0];
	auto v1 = _verts[i1];
	auto v2 = _verts[i2];

	_center = circumcenter(v0, v1, v2);

	quicksort(_ids, _verts, 0, n - 1, _center);

	_hull = new HullNode();
	_hull = InsertNode(_hull, _verts, i0, nullptr);
	// initialize a circular doubly-linked list that will hold an advancing convex hull
	_nodes.emplace_back(Node{ i0, _verts[i0], 0 });
	_nodes.emplace_back(Node{ i1, _verts[i1], 1 });
	_nodes.emplace_back(Node{ i2, _verts[i2], 2 });

	const int max_triangles = 2 * n - 5;
	_tris.resize(max_triangles * 3);
	_half_edges.resize(max_triangles * 3);
	_num_tris = 0;
	_AddTriangle(i0, i1, i2, -1, -1, -1);

	for (int k = 0, double xp, double yp; k < _ids.size(); k++) 
	{
		const int i = _ids[k];
		const WidePoint p = _verts[i];

		// skip duplicate points
		if (p.x == xp && p.y == yp) continue;
		xp = p.x;
		yp = p.y;

		// skip seed triangle points
		if ((p.x ==  _verts[i0].x && p.y == _verts[i0].y) ||
			(p.x ==  _verts[i1].x && p.y == _verts[i1].y) ||
			(p.x ==  _verts[i2].x && p.y == _verts[i2].y)) continue;

		// find a visible edge on the convex hull using edge hash
		const int startKey = _Hash(p);
		auto key = startKey;
		Node* start;
		do 
		{
			start = _hull[key];
			key = (key + 1) % _hash_size;
		} 
		while ((!start || start->removed) && key != startKey);

		e = start;
		while (area(x, y, e.x, e.y, e.next.x, e.next.y) >= 0) {
			e = e.next;
			if (e == = start) {
				throw new Error('Something is wrong with the input points.');
			}
		}
	}
}

int Triangulator::_Hash(const WidePoint& point)
{
	const double dx = point.x - _center.x;
	const double dy = point.y - _center.y;
	// use pseudo-angle: a measure that monotonically increases
	// with real angle, but doesn't require expensive trigonometry
	const double p = 1.0 - dx / (abs(dx) + abs(dy));
	return static_cast<int>(floor((2.0 + (dy < 0.0 ? -p : p)) / 4.0 * _hash_size));
}

void Triangulator::_Link(int a, int b) 
{
	_half_edges[a] = b;
	if (b != -1)
		_half_edges[b] = a;
}

// add a new triangle given vertex indices and adjacent half-edge ids
int Triangulator::_AddTriangle(int i0, int i1, int i2, int a, int b, int c) 
{
	const int t = _num_tris;

	_tris[t] = i0;
	_tris[t + 1] = i1;
	_tris[t + 2] = i2;

	_Link(t, a);
	_Link(t + 1, b);
	_Link(t + 2, c);

	_num_tris += 3;

	return t;
}
