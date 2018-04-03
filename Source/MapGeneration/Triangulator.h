#pragma once
#include <Core/StdIncludes.h>
#include <list>
#include <map>

struct WidePoint
{
	double x;
	double y;
};

struct HullNode
{
	int i;
	WidePoint p;
	int t;
	bool removed;
	std::shared_ptr<HullNode> prev;
	std::shared_ptr<HullNode> next;
};

class Triangulator
{

	std::vector<uint32_t> _ids;
	std::vector<WidePoint> _verts;
	WidePoint _center;
	int _hash_size;
	std::map<int, std::shared_ptr<HullNode>> _hashes;
	std::shared_ptr<HullNode> _hull;
	int _num_tris;
	std::vector<int> _tris;

	std::vector<int> _half_edges;
	void _Link(int a, int b);
	int _AddTriangle(int i0, int i1, int i2, int a, int b, int c);
	int _Hash(const WidePoint& point);
	void _HashNode(std::shared_ptr<HullNode> node);
	int _Legalize(int);
public:
	Triangulator(const std::vector<WidePoint>& verts);

	std::vector<int> GetTriangles() { return _tris; }
	std::vector<int> GetHalfEdges() { return _half_edges; }
};