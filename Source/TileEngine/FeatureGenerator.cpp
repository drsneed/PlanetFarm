#include "FeatureGenerator.h"
#include <random>
#include <deque>
#define JC_VORONOI_IMPLEMENTATION
//#define JCV_REAL_TYPE double
//#define JCV_FABS fabs
//#define JCV_ATAN2 atan2
#include <jc_voronoi.h>
//http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/
namespace
{
	std::vector<jcv_point> GenerateRandomPoints(float minimum, float maximum, int num_points, int seed)
	{
		//if no seed:
		//std::random_device randomDevice;
		//std::default_random_engine engine(randomDevice());
		std::default_random_engine randomizer(seed);
		std::uniform_real_distribution<float> random_value(minimum + 1.0f, maximum);
		std::vector<jcv_point> result(num_points);
		for (auto i = 0; i < num_points; ++i)
		{
			result[i] = jcv_point{ random_value(randomizer), random_value(randomizer) };
		}

		return result;
	}

	void RelaxPoints(const jcv_diagram* diagram, jcv_point* points)
	{
		const jcv_site* sites = jcv_diagram_get_sites(diagram);
		for (int i = 0; i < diagram->numsites; ++i)
		{
			const jcv_site* site = &sites[i];
			jcv_point sum = site->p;
			int count = 1;

			const jcv_graphedge* edge = site->edges;

			while (edge)
			{
				sum.x += edge->pos[0].x;
				sum.y += edge->pos[0].y;
				++count;
				edge = edge->next;
			}

			points[site->index].x = sum.x / count;
			points[site->index].y = sum.y / count;
		}
	}

	void DrawCells (const jcv_diagram* diagram)
	{
		// If you want to draw triangles, or relax the diagram,
		// you can iterate over the sites and get all edges easily
		const jcv_site* sites = jcv_diagram_get_sites(diagram);
		for (int i = 0; i < diagram->numsites; ++i)
		{
			const jcv_site* site = &sites[i];
			const jcv_graphedge* e = site->edges;
			while (e)
			{
				//draw_triangle(site->p, e->pos[0], e->pos[1]);
				e = e->next;
			}
		}
	}

	void DrawEdges(const jcv_diagram* diagram)
	{
		// If all you need are the edges
		const jcv_edge* edge = jcv_diagram_get_edges(diagram);
		while (edge)
		{
			//draw_line(edge->pos[0], edge->pos[1]);
			edge = edge->next;
		}
	}

	std::vector<XMFLOAT2> GetVertices(jcv_diagram* diagram)
	{
		std::deque<XMFLOAT2> vertices;
		const jcv_site* sites = jcv_diagram_get_sites(diagram);
		for (int i = 0; i < diagram->numsites; ++i)
		{
			const jcv_site* site = &sites[i];
			const jcv_graphedge* e = site->edges;
			bool is_edge_site = false;
			while (e)
			{
				auto edge_start = e->pos[0];
				auto edge_end = e->pos[1];

				if (is_edge_site)
				{
					if (e->neighbor != nullptr)
					{
						//TODO: Fix this bad logic. The edge neighbor could be another edge cell.
						// in this case, we would want ignore it. We only want the inner ring.

						// the edges with neighbors represent the interior ring of voronoi cells
						vertices.push_front(XMFLOAT2(edge_start.x, edge_start.y));
						vertices.push_front(XMFLOAT2(edge_end.x, edge_end.y));
					}
				}
				else if (edge_start.x == FEATURE_VERTEX_MIN || edge_end.x == FEATURE_VERTEX_MIN ||
					edge_start.x == FEATURE_VERTEX_MAX || edge_end.x == FEATURE_VERTEX_MAX ||
					edge_start.y == FEATURE_VERTEX_MIN || edge_end.y == FEATURE_VERTEX_MIN ||
					edge_start.y == FEATURE_VERTEX_MAX || edge_end.y == FEATURE_VERTEX_MAX)
				{
					is_edge_site = true;
					e = site->edges; // reset ptr to beginning of loop
					continue; // make sure to skip ptr setting below
				}
				e = e->next;
			}
		}

		std::vector<XMFLOAT2> result;

		while (vertices.size() > 0)
		{
			XMFLOAT2 edge_start = vertices.back();
			vertices.pop_back();
			XMFLOAT2 edge_end = vertices.back();
			vertices.pop_back();

			if (result.size() == 0)
			{
				result.push_back(edge_start);
				result.push_back(edge_end);
			}
			else
			{
				auto end = result.size() - 1;
				if (edge_start.x == result[end].x && edge_start.y == result[end].y)
				{
					result.push_back(edge_end);
				}
				else if (edge_end.x == result[end].x && edge_end.y == result[end].y)
				{
					result.push_back(edge_start);
				}
				else
				{
					// go to the end of the line
					vertices.push_front(edge_start);
					vertices.push_front(edge_end);
				}
			}
		}
		result.push_back(result[0]);
		return result;
	}
}


FeatureGenerator::FeatureGenerator(int seed)
	: _seed(seed)
{

}




Feature FeatureGenerator::GenerateIsland()
{
	int point_count = 16;
	auto points = GenerateRandomPoints(FEATURE_VERTEX_MIN, FEATURE_VERTEX_MAX, point_count, _seed);
	jcv_rect bounding_box
	{
		{FEATURE_VERTEX_MIN, FEATURE_VERTEX_MIN},
		{FEATURE_VERTEX_MAX, FEATURE_VERTEX_MAX}
	};
	jcv_diagram diagram = {};

	// two relaxations for now
	jcv_diagram_generate(point_count, &points[0], &bounding_box, &diagram);
	RelaxPoints(&diagram, &points[0]);
	jcv_diagram_free(&diagram);
	diagram = {};
	jcv_diagram_generate(point_count, &points[0], &bounding_box, &diagram);
	RelaxPoints(&diagram, &points[0]);
	jcv_diagram_free(&diagram);
	diagram = {};
	jcv_diagram_generate(point_count, &points[0], &bounding_box, &diagram);

	//TODO: Create feature from the diagram.
	auto vertices = GetVertices(&diagram);
	jcv_diagram_free(&diagram);

	return Feature(
		std::string("Voronoi Island"),
		Tile(0, 0, 0).GetID(), // tileid
		FeatureType::Unknown, // type
		XMFLOAT2(0.5f, 0.5f),
		0.0f, // rot
		vertices
	);
}