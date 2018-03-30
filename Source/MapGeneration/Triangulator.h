#pragma once
#include <Core/StdIncludes.h>

class Triangulator
{
	std::vector<MapPoint>& _input;
	std::vector<int> _triangles;
	std::vector<int> _half_edges;

	void _RemoveCollidingTriangles(const MapPoint& point, std::vector<int>& output, std::vector<int>& edges);
	void _DeleteDuplicateEdges(std::vector<int>& edges);
	void _DeleteInvalidTriangles(int maxValid, std::vector<int>& output);
	void _CalculateCircumCircle(int v1, int v2, int v3, MapPoint& center, float& radius);
public:
	Triangulator(std::vector<MapPoint>& input);

	std::vector<int> Triangles() { return _triangles; }
	std::vector<int> HalfEdges() { return _half_edges; }
};