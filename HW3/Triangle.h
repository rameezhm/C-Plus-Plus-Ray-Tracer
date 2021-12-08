#include <glm/glm.hpp>

#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

class Triangle {
public:
	glm::vec3 p1, p2, p3;
	glm::vec3 n; //triangle normal

	Triangle::Triangle() {
		p1, p2, p3, n = glm::vec3(0, 0, 0);
	}

	Triangle::Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
		p1 = v1;
		p2 = v2;
		p3 = v3;

		//compute normal
		glm::vec3 u = p1 - p2;
		glm::vec3 v = p3 - p1;

		n = normalize(glm::vec3(u.y * v.z - u.z * v.y, 
								u.z * v.x - u.x * v.z,
								u.x * v.y - u.y * v.x));
	}
};

#endif