#include "FeatureGenerator.h"
#include <random>
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
		std::uniform_real_distribution<float> random_value(minimum, maximum);
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
}


FeatureGenerator::FeatureGenerator(int seed)
	: _seed(seed)
{

}



Feature FeatureGenerator::GenerateIsland()
{
	int point_count = 4000;
	auto points = std::move(GenerateRandomPoints(FEATURE_VERTEX_MIN, FEATURE_VERTEX_MAX, point_count, _seed));
	jcv_rect bounding_box
	{ 
		{FEATURE_VERTEX_MIN, FEATURE_VERTEX_MIN}, 
		{FEATURE_VERTEX_MAX, FEATURE_VERTEX_MAX} 
	};
	jcv_diagram diagram = {};

	// two relaxations for now
	jcv_diagram_generate(point_count, &points[0], &bounding_box, &diagram);
	RelaxPoints(&diagram, &points[0]);
	jcv_diagram_generate(point_count, &points[0], &bounding_box, &diagram);
	RelaxPoints(&diagram, &points[0]);
	jcv_diagram_generate(point_count, &points[0], &bounding_box, &diagram);

	//TODO: Create feature from the diagram.


	jcv_diagram_free(&diagram);
}