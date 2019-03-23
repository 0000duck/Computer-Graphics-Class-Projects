#ifndef GRAPHICS_HEADERS_H
#define GRAPHICS_HEADERS_H

#include <iostream>

#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED

#include <GL/glew.h>

// GLM for matricies
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define INVALID_UNIFORM_LOCATION 0x7fffffff

struct Vertex {
	
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texcoord;

	Vertex() {}
	Vertex(glm::vec3 p, glm::vec3 n, glm::vec2 t): pos(p), normal(n), texcoord(t) {}
};

#endif /* GRAPHICS_HEADERS_H */
