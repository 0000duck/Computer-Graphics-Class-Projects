
#include "scene.h"

#include <assimp/Importer.hpp> 		//includes the importer, which is used to read our obj file
#include <assimp/scene.h> 			//includes the aiScene object
#include <assimp/postprocess.h> 	//includes the postprocessing variables for the importer
#include <assimp/color4.h> 			//includes the aiColor4 object, which is used to handle the colors from the Scene objects

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Scene::Scene() {}

Scene::Scene(std::string file) {
	Load(file);
}

Scene::~Scene() {
	Clear();
}

void Scene::Clear() {
	for(auto& m : meshes) {
		glDeleteBuffers(1, &m.VBO);
		glDeleteBuffers(1, &m.IBO);
	}
	for(auto& t : textures) {
		glDeleteTextures(1, &t.tex);
	}
	meshes.clear();
	textures.clear();
}

bool Scene::Load(std::string file) {

	std::string dir = file.substr(0, file.find_last_of("\\/") + 1);

	Clear();

	Assimp::Importer import;

	// load file
	const aiScene* scene = import.ReadFile(file.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
	if(!scene) {
		std::cerr << "Error reading file " << file << ": " << import.GetErrorString() << std::endl;
		return false;
	}

	const aiVector3D zeroVec(0.0f, 0.0f, 0.0f);

	// for each mesh in the file
	for(unsigned int mesh_idx = 0; mesh_idx < scene->mNumMeshes; mesh_idx++) {
		const aiMesh* aiM = scene->mMeshes[mesh_idx];

		Mesh m;
		m.textured = aiM->mMaterialIndex != 0;
		m.material = aiM->mMaterialIndex - 1;

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

			m.vertices.push_back(vert);
		}

		// for each face in the mesh
		for(unsigned int face_idx = 0; face_idx < aiM->mNumFaces; face_idx++) {
			const aiFace* face = &aiM->mFaces[face_idx];

			// collect face indicies
			for(unsigned int idx = 0; idx < 3; idx++) {
				m.indices.push_back(face->mIndices[idx]);
			}
		}

		// send vertex / index information to GPU
		glGenBuffers(1, &m.VBO);
		glGenBuffers(1, &m.IBO);

		glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.IBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m.vertices.size(), &m.vertices[0], GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m.indices.size(), &m.indices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		meshes.push_back(std::move(m));
	}

	// for each material
	for(unsigned int mat_idx = 0; mat_idx < scene->mNumMaterials; mat_idx++) {
		const aiMaterial* mat = scene->mMaterials[mat_idx];

		if(mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {

			aiString path;

			// if the diffuse (color) texture is there
			if(mat->GetTexture(aiTextureType_DIFFUSE, 0, &path, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {

				Texture t;

				// load file
				int w, h;
				std::string full_path = dir + path.C_Str();
				unsigned char* bitmap = stbi_load(full_path.c_str(), &w, &h, nullptr, 4);
				if(!bitmap) {
					std::cerr << "Failed to load texture from " << full_path.c_str() << std::endl;
					return false;
				}

				// Set up OpenGL texture
				glGenTextures(1, &t.tex);
				glBindTexture(GL_TEXTURE_2D, t.tex);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
				glGenerateMipmap(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);

				// free image loaded from file
				stbi_image_free(bitmap);

				textures.push_back(t);
			}
		}
	}

	return true;
}

void Scene::Render() {

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	for(auto& m : meshes) {

		glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.IBO);

		glActiveTexture(GL_TEXTURE0);
		if(m.textured) {
			glBindTexture(GL_TEXTURE_2D, textures[m.material].tex);
		}

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,normal));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,texcoord));

		glDrawElements(GL_TRIANGLES, m.indices.size(), GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}
