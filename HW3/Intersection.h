#include <glm/glm.hpp>
#include "Ray.h"
#include "Triangle.h"

#ifndef __INTERSECTION_H__
#define __INTERSECTION_H__

class Intersection {
public:
	Ray* ray; 
	Triangle* tri; 
	float dis;

	Intersection::Intersection() {
		dis = 0;
	}

	Intersection::Intersection(Ray* r, Triangle* t, float d) {
		ray = r;
		tri = t;
		dis = d;
	}

	// return point of intersection
	glm::vec3 Intersection::poi() {
		return ray->origin + dis * ray->dir;
	}
};

#endif
