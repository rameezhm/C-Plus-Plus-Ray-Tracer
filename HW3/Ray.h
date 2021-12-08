#include <glm/glm.hpp>

#ifndef __RAY_H__
#define __RAY_H__

class Ray {
public:
	glm::vec3 origin; //the origin of the ray
	glm::vec3 dir; //direction of ray

	Ray::Ray() {
		origin = glm::vec3(0, 0, 0);
		dir = glm::vec3(0, 0, 0);
	}

	Ray::Ray(glm::vec3 o, glm::vec3 d) {
		origin = o;
		dir = d;
	}
};

#endif