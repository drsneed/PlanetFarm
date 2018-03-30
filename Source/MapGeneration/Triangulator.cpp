#include "Triangulator.h"
#include <Core/DebugTools.h>
#include <cmath>
#include <algorithm>

void _CalculateSuperTriangleVertices(std::vector<MapPoint>& input, MapPoint& v1, MapPoint& v2, MapPoint& v3)
{
	ASSERT(input.size() > 0);

	auto minPt = input[0];
	auto maxPt = minPt;
	for (auto i = 1U; i < input.size(); ++i)
	{
		if (input[i].x < minPt.x) minPt.x = input[i].x;
		if (input[i].x > maxPt.x) maxPt.x = input[i].x;
		if (input[i].y < minPt.y) minPt.y = input[i].y;
		if (input[i].y > maxPt.y) maxPt.y = input[i].y;
	}

	// determine horizontal and vertical distance between min and max
	auto dx = maxPt.x - minPt.x;
	auto dy = maxPt.y - minPt.y;

	// record the largest distance
	auto dmax = max(dx, dy);

	// record midpoints
	auto xmid = (maxPt.x + minPt.x) / 2.0;
	auto ymid = (maxPt.y + minPt.y) / 2.0;

	v1.x = xmid - 20 * dmax;
	v1.y = ymid - dmax;
	v2.x = xmid;
	v2.y = ymid + 20 * dmax;
	v3.x = xmid + 20 * dmax;
	v3.y = ymid - dmax;
}


/// Collision test between a point and a circle. Returns true if point
/// is within the circle or lying on the circle's circumference
/// Note: using radius squared here
bool _Collision(const MapPoint& point, const MapPoint& center, const float radius)
{
	auto dx = point.x - center.x;
	auto dy = point.y - center.y;
	auto drsqr = dx*dx + dy*dy;
	return (drsqr - radius) <= FLT_EPSILON;
}

// Calculation of a circumcirle. Copied from http://paulbourke.net/papers/triangulate 
void _CalculateCircumCircle(const MapPoint& v1, const MapPoint& v2, const MapPoint& v3, MapPoint& center, float& radius)
{
	float m1, m2, mx1, mx2, my1, my2;
	float dx, dy;
	auto abs_y1y2 = fabs(v1.y - v2.y);
	auto abs_y2y3 = fabs(v2.y - v3.y);

	/* Check for coincident points */
	if (abs_y1y2 < FLT_EPSILON && abs_y2y3 < FLT_EPSILON)
		return;

	if (abs_y1y2 < FLT_EPSILON)
	{
		m2 = -(v3.x - v2.x) / (v3.y - v2.y);
		mx2 = (v2.x + v3.x) / 2.0;
		my2 = (v2.y + v3.y) / 2.0;
		center.x = (v2.x + v1.x) / 2.0;
		center.y = m2 * (center.x - mx2) + my2;
	}

	else if (abs_y2y3 < FLT_EPSILON)
	{
		m1 = -(v2.x - v1.x) / (v2.y - v1.y);
		mx1 = (v1.x + v2.x) / 2.0;
		my1 = (v1.y + v2.y) / 2.0;
		center.x = (v3.x + v2.x) / 2.0;
		center.y = m1 * (center.x - mx1) + my1;
	}
	else
	{
		m1 = -(v2.x - v1.x) / (v2.y - v1.y);
		m2 = -(v3.x - v2.x) / (v3.y - v2.y);
		mx1 = (v1.x + v2.x) / 2.0;
		mx2 = (v2.x + v3.x) / 2.0;
		my1 = (v1.y + v2.y) / 2.0;
		my2 = (v2.y + v3.y) / 2.0;
		center.x = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
		if (abs_y1y2 > abs_y2y3)
		{
			center.y = m1 * (center.x - mx1) + my1;
		}
		else
		{
			center.y = m2 * (center.x - mx2) + my2;
		}
	}
	dx = v2.x - center.x;
	dy = v2.y - center.y;
	radius = dx*dx + dy*dy;
}

void Triangulator::_CalculateCircumCircle(int v1, int v2, int v3, MapPoint& center, float& radius)
{
	::_CalculateCircumCircle(_input[v1], _input[v2], _input[v3], center, radius);
	radius = sqrt(radius);
}

void Triangulator::_DeleteDuplicateEdges(std::vector<int>& edges)
{
	auto count = edges.size();
	if (count <= 2)
	{
		return;
	}
	for (auto i = 0U; i < count - 2; i += 2)
	{
		for (auto j = i + 2; j < count; j += 2)
		{
			if (edges[i] == edges[j] && edges[i+1] == edges[j+1])
			{
				edges[i] = edges[i + 1] = edges[j] = edges[j + 1] = -1;
			}
		}
	}

	edges.erase(std::remove_if(edges.begin(), edges.end(),
		[](int& e) { return e == -1; }), edges.end());
}

void Triangulator::_DeleteInvalidTriangles(int maxValid, std::vector<int>& output)
{
	for (int i = 0; i < output.size(); ++i)
	{
		if (output[i] > maxValid)
			output[i] = -1;
	}
	output.erase(std::remove_if(output.begin(), output.end(),
		[](int& e) { return e == -1; }), output.end());
}

void Triangulator::_RemoveCollidingTriangles(const MapPoint& point, std::vector<int>& output, std::vector<int>& edges)
{
	for (int i = 0; i < output.size(); i += 3)
	{
		XMFLOAT2 center = {};
		float radius = 0.f;

		_CalculateCircumCircle(output[i], output[i+1], output[i+2], center, radius);
		if (_Collision(point, center, radius))
		{
			//(p1, p2)
			//(p2, p3)
			//(p3, p1)
			edges.push_back(output[i]);
			edges.push_back(output[i + 1]);
				
			edges.push_back(output[i + 1]);
			edges.push_back(output[i + 2]);
				
			edges.push_back(output[i + 2]);
			edges.push_back(output[i]);

			output[i] = -1;
			output[i + 1] = -1;
			output[i + 2] = -1;
		}
	}
	output.erase(std::remove_if(output.begin(), output.end(),
		[](int& e) { return e == -1; }), output.end());
}



Triangulator::Triangulator(std::vector<MapPoint>& input)
	: _input(input)
{
	int pointCount = static_cast<int>(input.size());
	std::vector<int> output;

	// Less than 3 = no triangles
	if (pointCount < 3) return;

	MapPoint super_tri_1, super_tri_2, super_tri_3;
	_CalculateSuperTriangleVertices(input, super_tri_1, super_tri_2, super_tri_3);
	input.push_back(super_tri_1);
	input.push_back(super_tri_2);
	input.push_back(super_tri_3);

	// Append a new triangle (the super triangle) indices to the output list
	output.push_back(pointCount);
	output.push_back(pointCount+1);
	output.push_back(pointCount+2);
	

	std::vector<int> edges;

	// Add each input point to the mesh and recalculate mesh
	for (auto i = 0U; i < pointCount; ++i)
	{
		// Some triangles in the existing mesh need to be removed to make space for new triangles
		_RemoveCollidingTriangles(input[i], output, edges);

		// Remove duplicate edges from the edge buffer, leaving an enclosing polygon
		_DeleteDuplicateEdges(edges);

		// Create a new triangle between each edge and the new vertex
		for (int i = 0; i < edges.size(); i += 2)
		{
			output.push_back(edges[i]);
			output.push_back(edges[i + 1]);
			output.push_back(i);
		}

		// Wipe this baby clean in preparation for the next iteration
		edges.clear();
	}

	// Delete any triangles that reference a super triangle vertex
	_DeleteInvalidTriangles(pointCount - 1, output);

	// Shrink input back to original size
	input.resize(pointCount);
}