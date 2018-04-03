#pragma once
#include <Core/StdIncludes.h>
#include <list>
#include <map>

struct WidePoint
{
	double x;
	double y;
};

class Triangulator
{
	std::vector<uint32_t> _ids;
	std::vector<WidePoint> _verts;
	WidePoint _center;
	int _hash_size;
	std::map<int, HullNode*> _hashes;
	HullNode* _hull;
	int _num_tris;
	std::vector<int> _tris;

	std::vector<int> _half_edges;
	void _Link(int a, int b);
	int _AddTriangle(int i0, int i1, int i2, int a, int b, int c);
	int _Hash(const WidePoint& point);
public:
	Triangulator(const std::vector<WidePoint>& verts);
};