
#ifndef MESH_H
#define MESH_H

#include <vector>
#include "graphics_headers.h"

class Scene {

public:
	Scene();
	Scene(std::string file);
	~Scene();

	bool Load(std::string file);
	void Render();
	void Clear();

private:

	struct Mesh {

		GLuint VBO, IBO;

		std::vector<Vertex> 		vertices;
		std::vector<unsigned int> 	indices; 

		unsigned int material;
	};

	std::vector<Mesh> meshes;
};

#endif // MESH_H
