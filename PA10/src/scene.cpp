
#include "scene.h"

#include <assimp/Importer.hpp> 		//includes the importer, which is used to read our obj file
#include <assimp/scene.h> 			//includes the aiScene object
#include <assimp/postprocess.h> 	//includes the postprocessing variables for the importer
#include <assimp/color4.h> 			//includes the aiColor4 object, which is used to handle the colors from the Scene objects
#include <stb_image.h>

Scene::Scene() {
	mesh.VBO = mesh.IBO = texture.handle = 0;
}

Scene::~Scene() {}

void Scene::DeleteMesh() {
	mesh.vertices.clear();
	mesh.indices.clear();
	if(mesh.VBO) {
		glDeleteBuffers(1, &mesh.VBO);
		mesh.VBO = 0;
	}
	if(mesh.IBO) {
		glDeleteBuffers(1, &mesh.IBO);
		mesh.IBO = 0;
	}
}

void Scene::DeleteTexture() {
	if(texture.handle) {
		glDeleteTextures(1, &texture.handle);
		texture.handle = 0;
	}
}

Scene::Mesh& Scene::getMesh() {

	return mesh;
}

bool Scene::LoadModel(std::string file, int idx) {

	std::string dir = file.substr(0, file.find_last_of("\\/") + 1);

	DeleteMesh();

	Assimp::Importer import;

	// load file
	const aiScene* scene = import.ReadFile(file.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs);
	if(!scene) {
		std::cerr << "Error reading file " << file << ": " << import.GetErrorString() << std::endl;
		return false;
	}

	const aiVector3D zeroVec(0.0f, 0.0f, 0.0f);

	// load first mesh
	unsigned int mesh_idx = idx;
	const aiMesh* aiM = scene->mMeshes[mesh_idx];

	// for each vertex in the mesh
	for(unsigned int vert_idx = 0; vert_idx < aiM->mNumVertices; vert_idx++) {
		const aiVector3D* pos = &aiM->mVertices[vert_idx];
		const aiVector3D* norm = &aiM->mNormals[vert_idx];
		const aiVector3D* texcoord = aiM->HasTextureCoords(0) ? &aiM->mTextureCoords[0][vert_idx] : &zeroVec;

		// translate vetex data
		Vertex vert;
		vert.pos = glm::vec3(pos->x, pos->y, pos->z);
		vert.normal = glm::vec3(norm->x, norm->y, norm->z);
		vert.texcoord = glm::vec2(texcoord->x, texcoord->y);

		mesh.vertices.push_back(vert);
	}

	// for each face in the mesh
	for(unsigned int face_idx = 0; face_idx < aiM->mNumFaces; face_idx++) {
		const aiFace* face = &aiM->mFaces[face_idx];

		// collect face indicies
		for(unsigned int idx = 0; idx < 3; idx++) {
			mesh.indices.push_back(face->mIndices[idx]);
		}
	}

	// send vertex / index information to GPU
	glGenBuffers(1, &mesh.VBO);
	glGenBuffers(1, &mesh.IBO);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh.vertices.size(), &mesh.vertices[0], GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh.indices.size(), &mesh.indices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

bool Scene::LoadTexture(std::string file) {

	DeleteTexture();

	int w, h;
	unsigned char* bitmap = stbi_load(file.c_str(), &w, &h, nullptr, 4);
	if(!bitmap) {
		std::cerr << "Failed to load texture from " << file.c_str() << std::endl;
		return false;
	}

	// Set up OpenGL texture
	glGenTextures(1, &texture.handle);
	glBindTexture(GL_TEXTURE_2D, texture.handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	// free image loaded from file
	stbi_image_free(bitmap);

	return true;
}

void Scene::Render() {

	if(!mesh.vertices.size() || !mesh.indices.size()) return;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IBO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.handle);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,texcoord));

	glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}
