#include <glm/glm.hpp>
#include "Ray.h"
#include "Triangle.h"

#ifndef __INTERSECTION_H__
#define __INTERSECTION_H__

class Intersection {
public:
	Ray* ray; 
	Triangle* tri; 
	float dis; // distance from ray origin to point of intersection along path of ray
	bool in = false; // is intersection valid/inside triangle

	Intersection::Intersection() {
		dis = 0;
		in = false;
	}

	Intersection::Intersection(Ray* r, Triangle* t, float d, bool i) {
		ray = r;
		tri = t;
		dis = d;
		in = i;
	}

	// return point of intersection
	glm::vec3 Intersection::poi() {
		return ray->origin + dis * ray->dir;
	}

};

#endif
