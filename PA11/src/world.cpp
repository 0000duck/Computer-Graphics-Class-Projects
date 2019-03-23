
#include "world.h"
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <stb_image.h>
#include <algorithm>
#include <imgui.h>
#include <thread>
#include <stb_perlin.h>

bool isRegularFile(std::string path);

Chunk::position::position(int _x, int _z) {
	x = _x;
	z = _z;
}

bool Chunk::position::operator==(const Chunk::position& other) const {
	return x == other.x && z == other.z;
}

Chunk::Chunk(btDiscreteDynamicsWorld * world) {
	generating = true;
	btWorld = world;
}

Chunk::~Chunk() {
	if(VBO) {
		glDeleteBuffers(1, &VBO);
	}
	if(VAO) {
		glDeleteVertexArrays(1, &VAO);
	}

	if(hasPhysics) DeletePhysics();
}

void Chunk::Render(ShaderInfo info, const std::vector<light>& near_lights) {

	if(ogl_refresh) {

		OpenGL();
		ogl_refresh = false;
	}

	GLint num_lights_loc, ambient_color_loc, model_loc;
	num_lights_loc = info.shader->GetUniformLocation("num_lights");
	ambient_color_loc = info.shader->GetUniformLocation("ambient_color");
	model_loc = info.shader->GetUniformLocation("model");

	GLint obj_ambient_loc, obj_diffuse_loc, obj_specular_loc, obj_shine_loc;
	obj_ambient_loc = info.shader->GetUniformLocation("object.ambient");
	obj_diffuse_loc = info.shader->GetUniformLocation("object.diffuse");
	obj_specular_loc = info.shader->GetUniformLocation("object.specular");
	obj_shine_loc = info.shader->GetUniformLocation("object.shine");

	int num_lights = 0;
	for(const light& l : near_lights) {
		if(num_lights >= 32) break;

		std::string light = "lights[" + std::to_string(num_lights) + "].";

		glUniform4fv(info.shader->GetUniformLocation((light + "pos").c_str()), 1, glm::value_ptr(l.pos));
		glUniform3fv(info.shader->GetUniformLocation((light + "diffuse_color").c_str()), 1, l.pos.w != 0 ? glm::value_ptr(info.diffuse_light) : glm::value_ptr(glm::vec3(0.5f)));
		glUniform3fv(info.shader->GetUniformLocation((light + "specular_color").c_str()), 1, glm::value_ptr(info.specular_light));
		glUniform1f(info.shader->GetUniformLocation((light + "constant_attenuation").c_str()), info.const_atten);
		glUniform1f(info.shader->GetUniformLocation((light + "linear_attenuation").c_str()), info.lin_atten);
		glUniform1f(info.shader->GetUniformLocation((light + "quadratic_attenuation").c_str()), info.quad_atten);
		glUniform3fv(info.shader->GetUniformLocation((light + "spotlight_direction").c_str()), 1, glm::value_ptr(info.spot_dir));
		glUniform1f(info.shader->GetUniformLocation((light + "spotlight_cutoff").c_str()), info.spot_cutoff);
		glUniform1f(info.shader->GetUniformLocation((light + "spotlight_exponent").c_str()), info.spot_exp);

		num_lights++;
	}

	glUniform1i(num_lights_loc, num_lights);

	glUniform3fv(ambient_color_loc, 1, glm::value_ptr(info.ambient_light));
	glUniform3fv(obj_ambient_loc, 1, glm::value_ptr(glm::vec4(1.0f)));
	glUniform3fv(obj_diffuse_loc, 1, glm::value_ptr(glm::vec4(1.0f)));
	glUniform3fv(obj_specular_loc, 1, glm::value_ptr(glm::vec4(0.0f)));
	glUniform1f(obj_shine_loc, 1.0f);

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x * CHUNK_SIZE_XZ, 0, pos.z * CHUNK_SIZE_XZ));
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(transform));

	glBindVertexArray(VAO);
	if(info.wireframe) {
		glDrawArrays(GL_LINES, 0, buffered_quads * 6);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, buffered_quads * 6);
	}
	glBindVertexArray(0);
}

void Chunk::Generate() {

	for(int x = 0; x < CHUNK_SIZE_XZ; x++) {
		for(int z = 0; z < CHUNK_SIZE_XZ; z++) {

			float fx = pos.x + x / 16.0f;
			float fz = pos.z + z / 16.0f;

			float f = stb_perlin_noise3(fx / 8.0f, 0, fz / 8.0f, 0, 0, 0);

			f = f * CHUNK_SIZE_Y / 2.0f + CHUNK_SIZE_Y / 2.0f;

			int y_max = (int)f;
			for(int i = 0; i <= y_max; i++) {
				if(i == 0) {
					blocks[x][z][i].texture = 4;
				} else if(i >= 1 && i <= y_max - 5) {
					blocks[x][z][i].texture = 3;
				}
				if(y_max > 93) {
					if(i >= y_max - 4 && i <= y_max - 1) {
						blocks[x][z][i].texture = 1;
					} else if(i == y_max) {
						blocks[x][z][i].texture = 0;
					}
				} else {
					if(i >= y_max - 4) {
						blocks[x][z][i].texture = 2;
					}
				}
			}
		}
	}
}

bool Chunk::Occupied(int x, int y, int z) {

	if(y < 0 || y >= CHUNK_SIZE_Y) return false;
	if(x >= 0 && z >= 0 && x < CHUNK_SIZE_XZ && z < CHUNK_SIZE_XZ) {
		return blocks[x][z][y].texture != 255;
	}
	return false;
}

void Chunk::OpenGL() {

	if(!VAO) {
		glGenVertexArrays(1, &VAO);
	}
	if(VBO) {
		glDeleteBuffers(1, &VBO);
	}
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(vertex), mesh.data(), GL_STATIC_DRAW);
	mesh.clear();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
}

void Chunk::AddQuad(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, size_t width, size_t height, block type, glm::vec3 normal) {

	float tx = (float)type.texture;

    vertex vertices[] = {
        { v0, { 0, 0, tx }, normal },
        { v1, { (float) width, 0, tx }, normal },
        { v2, { 0, (float) height, tx }, normal },
        { v3, { (float) width, (float) height, tx }, normal },
    };

		/*int chunkPosX = pos.x * CHUNK_SIZE_XZ;
		int chunkPosZ = pos.z * CHUNK_SIZE_XZ;

		btVector3 bv0 = btVector3(v0.x + chunkPosX, v0.y, v0.z + chunkPosZ);
		btVector3 bv1 = btVector3(v1.x + chunkPosX, v1.y, v1.z + chunkPosZ);
		btVector3 bv2 = btVector3(v2.x + chunkPosX, v2.y, v2.z + chunkPosZ);
		btVector3 bv3 = btVector3(v3.x + chunkPosX, v3.y, v3.z + chunkPosZ);

		btMesh->addTriangle(bv0, bv1, bv2);
		btMesh->addTriangle(bv1, bv2, bv3);*/

    mesh.insert(mesh.end(), vertices, vertices + 3);
    mesh.insert(mesh.end(), vertices + 1, vertices + 4);

    buffered_quads++;
}

// adapted from the implementation for https://github.com/darkedge/starlight
void Chunk::Build() {

	mesh.clear();

	block slice[CHUNK_SIZE_XZ * CHUNK_SIZE_Y];

	// current position
	int xyz[] = { 0, 0, 0 };
	int max[] = { CHUNK_SIZE_XZ, CHUNK_SIZE_Y, CHUNK_SIZE_XZ };

	for (int i = 0; i < 6; i++) {

		int d0 = (i + 0) % 3;
		int d1 = (i + 1) % 3;
		int d2 = (i + 2) % 3;
		int backface = i / 3 * 2 - 1;

		// Traverse the chunk
		for (xyz[d0] = 0; xyz[d0] < max[d0]; xyz[d0]++) {

			// Fill in slice
			for (xyz[d1] = 0; xyz[d1] < max[d1]; xyz[d1]++) {
				for (xyz[d2] = 0; xyz[d2] < max[d2]; xyz[d2]++) {
					if(xyz[0] >= 0 && xyz[0] < CHUNK_SIZE_XZ && xyz[1] >= 0 && xyz[1] < CHUNK_SIZE_Y && xyz[2] >=0 && xyz[2] < CHUNK_SIZE_XZ) {
						block b = blocks[xyz[0]][xyz[2]][xyz[1]];

						// check for air
						if (b.texture != 255) {
							// Check neighbor
							xyz[d0] += backface;
							if(xyz[0] >= 0 && xyz[0] < CHUNK_SIZE_XZ && xyz[1] >= 0 && xyz[1] < CHUNK_SIZE_Y && xyz[2] >=0 && xyz[2] < CHUNK_SIZE_XZ) {
								if (blocks[xyz[0]][xyz[2]][xyz[1]].texture != 255) {
									slice[xyz[d1] * max[d2] + xyz[d2]].texture = 255;
								} else {
									slice[xyz[d1] * max[d2] + xyz[d2]].texture = b.texture;
								}
							} else {
								slice[xyz[d1] * max[d2] + xyz[d2]].texture = b.texture;
							}
							xyz[d0] -= backface;
						} else {
							slice[xyz[d1] * max[d2] + xyz[d2]].texture = 255;
						}
					}
				}
			}

			// Mesh the slice
			for (xyz[d1] = 0; xyz[d1] < max[d1]; xyz[d1]++) {
				for (xyz[d2] = 0; xyz[d2] < max[d2];) {
					block type = slice[xyz[d1] * max[d2] + xyz[d2]];

					// check for air
					if (type.texture == 255) {
						xyz[d2]++;
						continue;
					}

					int width = 1;

					// Find the largest line
					for (int d22 = xyz[d2] + 1; d22 < max[d2]; d22++) {
						if (slice[xyz[d1] * max[d2] + d22].texture != type.texture) break;
						width++;
					}

					int height = 1;

					// Find the largest rectangle
					bool done = false;
					for (int d11 = xyz[d1] + 1; d11 < max[d1]; d11++) {
						// Find lines of the same width
						for (int d22 = xyz[d2]; d22 < xyz[d2] + width; d22++) {
							if (slice[d11 * max[d2] + d22].texture != type.texture) {
								done = true;
								break;
							}
						}
						if (done) break;
						height++;
					}

					float w[] = { 0, 0, 0 };
					w[d2] = (float) width;
					float h[] = { 0, 0, 0 };
					h[d1] = (float) height;

					glm::vec3 v {(float) xyz[0], (float) xyz[1], (float) xyz[2]};

					// shift front faces by one block
					if (backface > 0) {
						float f[] = { 0, 0, 0 };
						f[d0] += 1.0f;
						v += glm::vec3{ f[0], f[1], f[2] };
					}

					// emit quad
					switch (i) {
					case 0: // -X
						AddQuad(v, v + glm::vec3{ w[0], w[1], w[2] },
							v + glm::vec3{ h[0], h[1], h[2] },
							v + glm::vec3{ w[0] + h[0], w[1] + h[1], w[2] + h[2] },
							width, height, type, glm::vec3(-1, 0, 0));
						break;
					case 1: // -Y
						AddQuad(v, v + glm::vec3{ w[0], w[1], w[2] },
							v + glm::vec3{ h[0], h[1], h[2] },
							v + glm::vec3{ w[0] + h[0], w[1] + h[1], w[2] + h[2] },
							width, height, type, glm::vec3(0, -1, 0));
						break;
					case 2: // -Z
						AddQuad(v + glm::vec3{ h[0], h[1], h[2] }, v,
							v + glm::vec3{ w[0] + h[0], w[1] + h[1], w[2] + h[2] },
							v + glm::vec3{ w[0], w[1], w[2] },
							height, width, type, glm::vec3(0, 0, 1));
						break;
					case 3: // +X
						AddQuad(v + glm::vec3{ w[0], w[1], w[2] }, v,
							v + glm::vec3{ w[0] + h[0], w[1] + h[1], w[2] + h[2] },
							v + glm::vec3{ h[0], h[1], h[2] },
							width, height, type, glm::vec3(1, 0, 0));
						break;
					case 4: // +Y
						AddQuad(v + glm::vec3{ h[0], h[1], h[2] },
							v + glm::vec3{ w[0] + h[0], w[1] + h[1], w[2] + h[2] },
							v, v + glm::vec3{ w[0], w[1], w[2] }, width, height, type, glm::vec3(0, 1, 0));
						break;
					case 5: // +Z
						AddQuad(v, v + glm::vec3{ h[0], h[1], h[2] },
							v + glm::vec3{ w[0], w[1], w[2] },
							v + glm::vec3{ w[0] + h[0], w[1] + h[1], w[2] + h[2] },
							height, width, type, glm::vec3(0, 0, -1));
						break;
					}

					// Zero the quad in the slice
					for (int d11 = xyz[d1]; d11 < xyz[d1] + height; d11++) {
						for (int d22 = xyz[d2]; d22 < xyz[d2] + width; d22++) {
							slice[d11 * max[d2] + d22].texture = 255;
						}
					}

					// Advance search position for next quad
					xyz[d2] += width;
				}
			}
		}
	}

	ogl_refresh = true;
}

void Chunk::AddLight(float x, float y, float z) {

	light l;
	l.pos = glm::vec4(x, y, z, 1);
	lights.push_back(l);
}

void Chunk::SetPhysics() {
	btShape = new btStaticPlaneShape(btVector3(0, -1, 0), CHUNK_SIZE_XZ);
	btMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(pos.x, CHUNK_SIZE_Y, pos.z)));

	btInertia = btVector3(0,0,0);
	btShape->calculateLocalInertia(0, btInertia);

	btRigidBody::btRigidBodyConstructionInfo btBodyCTI(0, btMotionState, btShape, btInertia);
	btBody = new btRigidBody(btBodyCTI);
	//btBody->setActivationState(DISABLE_DEACTIVATION);

	btWorld->addRigidBody(btBody);
	hasPhysics = true;
}

void Chunk::DeletePhysics() {
	if(btBody) {
		btWorld->removeRigidBody(btBody);
		delete btBody;
	}
	if(btShape) delete btShape;
	if(btMotionState) delete btMotionState;
	//if(btMesh) delete btMesh;

	btWorld = nullptr;
	btShape = nullptr;
	btMotionState = nullptr;
	btBody = nullptr;
	//btMesh = nullptr;

	hasPhysics = false;
}

World::World(FreeCamera* c, int* _w, int* _h) : pool(4) {

	cam = c;
	w = _w;
	h = _h;

	glGenTextures(1, &textures);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textures);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 32, 32, 255);
	LoadTextures();
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_REPEAT);

	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	solver = new btSequentialImpulseConstraintSolver();

	btWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	btWorld->setGravity(btVector3(0, 0, 0));

	btWorld->addRigidBody(cam->btBody);
}

void World::TryPlace() {

	for(float i = 4; i > 0; i -= 0.25) {
		glm::vec3 player_forward = cam->pos + glm::normalize(cam->front) * glm::vec3(i);

		int x = (int)player_forward.x % CHUNK_SIZE_XZ;
		int z = (int)player_forward.z % CHUNK_SIZE_XZ;
		while(x < 0) x += CHUNK_SIZE_XZ;
		while(z < 0) z += CHUNK_SIZE_XZ;
		int y = (int)player_forward.y;

		int cx = player_forward.x / CHUNK_SIZE_XZ - (player_forward.x < 0 ? 1 : 0);
		int cz = player_forward.z / CHUNK_SIZE_XZ - (player_forward.z < 0 ? 1 : 0);

		auto chunk = chunks.find(Chunk::position(cx, cz));

		if(chunk->second->blocks[x][z][y].texture == 255) {

			chunk->second->blocks[x][z][y].texture = select;
			if(select == 5) {
				chunk->second->AddLight(x + cx * CHUNK_SIZE_XZ + 0.5, y + 0.5, z + cz * CHUNK_SIZE_XZ + 0.5);
			}

			Chunk* c = chunk->second;
			pool.enqueue([c]() -> void {

				c->Build();
			});

			break;
		}
	}
}

void World::TryDestroy() {

	for(float i = 0; i < 5; i += 0.25) {
		glm::vec3 player_forward = cam->pos + cam->front * glm::vec3(i);

		int x = (int)player_forward.x % CHUNK_SIZE_XZ;
		int z = (int)player_forward.z % CHUNK_SIZE_XZ;
		while(x < 0) x += CHUNK_SIZE_XZ;
		while(z < 0) z += CHUNK_SIZE_XZ;
		int y = (int)player_forward.y;

		int cx = player_forward.x / CHUNK_SIZE_XZ - (player_forward.x < 0 ? 1 : 0);
		int cz = player_forward.z / CHUNK_SIZE_XZ - (player_forward.z < 0 ? 1 : 0);

		auto chunk = chunks.find(Chunk::position(cx, cz));
		if(chunk->second->blocks[x][z][y].texture != 255 && chunk->second->blocks[x][z][y].texture != 4) {

			if(chunk->second->blocks[x][z][y].texture == 5) {
				for(int i = 0; i < (int)chunk->second->lights.size(); i++) {
					Chunk::light l = chunk->second->lights[i];

					if(l.pos.x == x + cx * CHUNK_SIZE_XZ + 0.5 &&
					   l.pos.y == y + 0.5 &&
					   l.pos.z == z + cz * CHUNK_SIZE_XZ + 0.5) {

						chunk->second->lights.erase(chunk->second->lights.begin() + i);
					}
				}
			}

			chunk->second->blocks[x][z][y].texture = 255;

			Chunk* c = chunk->second;
			pool.enqueue([c]() -> void {

				c->Build();
			});

			break;
		}
	}
}

std::vector<Chunk::light> World::GetLights(Chunk* c) {

	std::vector<Chunk::light> ret = c->lights;

	Chunk::position pos = c->pos;

	const int radius = 2;
	for(int i = pos.x - radius; i <= pos.x + radius; i++) {
		for(int j = pos.z - radius; j <= pos.z + radius; j++) {
			if(!(i == pos.x && j == pos.z)) {

				auto c = chunks.find(Chunk::position(i, j));
				if(c != chunks.end()) ret.insert(ret.end(), c->second->lights.begin(), c->second->lights.end());
			}
		}
	}

	Chunk::light l;
	l.pos = glm::vec4(-1, -1, -1, 0);

	ret.push_back(l);

	return ret;
}

void World::AddLight() {

	Chunk::position camChunk = GetCameraChunk();
	auto chunk = chunks.find(camChunk);

	if(chunk != chunks.end()) {

		chunk->second->AddLight(cam->pos.x, cam->pos.y, cam->pos.z);
	}
}

void World::LoadTextures() {

	DIR *directory;
	dirent *entry;
	std::string dirPath = "../data/textures";

	if (dirPath.back() != '/' && dirPath.back() != '\\') {
		dirPath.append("/");
	}

	directory = opendir(dirPath.c_str());

	std::vector<std::string> files_alphabetical;
	while((entry = readdir(directory))) {
		std::string entryName = entry->d_name;

		if(entryName != ".." && entryName != "." && isRegularFile(dirPath + entryName)) {
			files_alphabetical.push_back(entryName);
		}
	}
	closedir(directory);

	std::sort(files_alphabetical.begin(), files_alphabetical.end());

	int index = 0;
	for(auto& entryName : files_alphabetical) {
		std::cout << "Loading texture "<< entryName << std::endl;

		if(!LoadTexture(dirPath + entryName, index)) {
			std::cerr << "Failed to load texture from file " << entryName << std::endl;
		} else {
			index++;
		}
	}
}

bool World::LoadTexture(std::string file, int index) {

	int w, h;
	unsigned char* bitmap = stbi_load(file.c_str(), &w, &h, nullptr, 4);
	if(!bitmap) return false;

	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);

	GLuint copy_because_imgui;
	glGenTextures(1, &copy_because_imgui);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, copy_because_imgui);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	textures_as_a_list.push_back(copy_because_imgui);

	stbi_image_free(bitmap);

	return true;
}

World::~World() {

	glDeleteTextures(1, &textures);
	for(auto t : textures_as_a_list) {
		glDeleteTextures(1, &t);
	}

	for(auto& c : chunks) {
		delete c.second;
	}

	if(btWorld) delete btWorld;
	if(solver) delete solver;
	if(dispatcher) delete dispatcher;
	if(collisionConfiguration) delete collisionConfiguration;
	if(broadphase) delete broadphase;

	broadphase = nullptr;
	collisionConfiguration = nullptr;
	dispatcher = nullptr;
	solver = nullptr;
	btWorld = nullptr;
}

void World::Scroll(int y) {

	select -= y;
	if(select < 0) select = 0;
	if(select >= (int)textures_as_a_list.size()) select = textures_as_a_list.size() - 1;
}

void World::UI() {

	int num_chunks = 0, num_quads = 0;
	for(Chunk* c : viewable) {
		num_quads += c->buffered_quads;
		num_chunks++;
	}

	ImGui::Begin("Menu");

	if(ImGui::CollapsingHeader("Lighting")) {
		ImGui::Indent();
		if(ImGui::CollapsingHeader("Ambient Light")) {
			ImGui::PushID(0);
			ImGui::Indent();
			ImGui::SliderFloat("R", &ambient_light.x, 0.0f, 1.0f);
			ImGui::SliderFloat("G", &ambient_light.y, 0.0f, 1.0f);
			ImGui::SliderFloat("B", &ambient_light.z, 0.0f, 1.0f);
			ImGui::Unindent();
			ImGui::PopID();
		}
		if(ImGui::CollapsingHeader("Diffuse Lights")) {
			ImGui::PushID(1);
			ImGui::Indent();
			ImGui::SliderFloat("R", &diffuse_light.x, 0.0f, 1.0f);
			ImGui::SliderFloat("G", &diffuse_light.y, 0.0f, 1.0f);
			ImGui::SliderFloat("B", &diffuse_light.z, 0.0f, 1.0f);
			ImGui::Unindent();
			ImGui::PopID();
		}
		if(ImGui::CollapsingHeader("Specular Lights")) {
			ImGui::PushID(2);
			ImGui::Indent();
			ImGui::SliderFloat("R", &specular_light.x, 0.0f, 1.0f);
			ImGui::SliderFloat("G", &specular_light.y, 0.0f, 1.0f);
			ImGui::SliderFloat("B", &specular_light.z, 0.0f, 1.0f);
			ImGui::Unindent();
			ImGui::PopID();
		}
		if(ImGui::CollapsingHeader("Spotlight Direction")) {
			ImGui::PushID(3);
			ImGui::Indent();
			ImGui::SliderFloat("X", &spot_dir.x, -1.0f, 1.0f);
			ImGui::SliderFloat("Y", &spot_dir.y, -1.0f, 1.0f);
			ImGui::SliderFloat("Z", &spot_dir.z, -1.0f, 1.0f);
			ImGui::Unindent();
			ImGui::PopID();
		}
		ImGui::SliderFloat("Const Attenuation", &const_atten, 0.0f, 1.0f);
		ImGui::SliderFloat("Linear Attenuation", &lin_atten, 0.0f, 1.0f);
		ImGui::SliderFloat("Quadratic Attenuation", &quad_atten, 0.0f, 1.0f);
		ImGui::SliderFloat("Spotlight Cutoff", &spot_cutoff, 0.0f, 90.0f);
		ImGui::SliderFloat("Spotlight Exponent", &spot_exp, 0.0f, 10.0f);
		ImGui::Unindent();
	}

	ImGui::Checkbox("Wireframe", &draw_wireframe);
	ImGui::Checkbox("Third Person", &cam->third_person);

	ImGui::Text("Chunks: %d", num_chunks);
	ImGui::Text("Quads: %d", num_quads);
	ImGui::Text("Camera: %f %f %f", cam->pos.x, cam->pos.y, cam->pos.z);
	ImGui::SliderInt("View Distance: ", &view_distance, 0, 16);
	ImGui::End();

	ImGui::SetNextWindowPos({(float)*w / 2 - 175, (float)*h - 85});
	ImGui::SetNextWindowSize({350, 85});
	ImGui::Begin("Blocks", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);

	for(int i = 0; i < (int)textures_as_a_list.size(); i++) {
		if(i == select) {
			ImGui::Dummy({10, 0});
			ImGui::SameLine();
			ImGui::Text("\\/");
			ImGui::SameLine();
			ImGui::Dummy({10, 0});
			ImGui::SameLine();
		} else {
			ImGui::Dummy({50, 0});
			ImGui::SameLine();
		}
	}
	ImGui::NewLine();
	for(auto t : textures_as_a_list) {
		ImGui::Image((ImTextureID)(Uint64)t, {50, 50});
		ImGui::SameLine();
	}
	ImGui::End();
}

void World::StartGenerating() {

	for(int i = -view_distance; i <= view_distance; i++) {
		for(int j = -view_distance; j <= view_distance; j++) {

			Chunk* c = new Chunk(btWorld);
			c->pos.x = i;
			c->pos.z = j;
			chunks.insert({c->pos, c});

			pool.enqueue([c]() -> void {

				c->Generate();
				c->Build();
				c->generating = false;
			});
		}
	}
}

Chunk::position World::GetCameraChunk() {

	Chunk::position ret;

	float x = cam->pos.x;
	float z = cam->pos.z;

	ret.x = x / CHUNK_SIZE_XZ - (x < 0 ? 1 : 0);
	ret.z = z / CHUNK_SIZE_XZ - (z < 0 ? 1 : 0);

	return ret;
}

bool World::PointViewable(glm::vec3 point) {

	point -= cam->pos;
	point = glm::normalize(point);
	float angle = acos(glm::dot(point, glm::normalize(glm::vec3(cam->front.x, 0, cam->front.z))));
	float factor = std::max((cam->pos.y - 50.0f) / 50.0f, 1.5f);

	return angle >= -factor * glm::radians(cam->fov) && angle <= factor * glm::radians(cam->fov);
}

void World::GetViewable() {

	viewable.clear();

	Chunk::position camChunk = GetCameraChunk();

	for(int i = camChunk.x - view_distance; i <= camChunk.x + view_distance; i++) {
		for(int j = camChunk.z - view_distance; j <= camChunk.z + view_distance; j++) {

			Chunk::position pos(i,j);

			auto chunk = chunks.find(pos);
			if(chunk != chunks.end()) {

				if(!chunk->second->generating) {

					bool see = false;

					see = see || PointViewable(glm::vec3(i * CHUNK_SIZE_XZ, 0, j * CHUNK_SIZE_XZ));
					see = see || PointViewable(glm::vec3(i * CHUNK_SIZE_XZ + CHUNK_SIZE_XZ, 0, j * CHUNK_SIZE_XZ));
					see = see || PointViewable(glm::vec3(i * CHUNK_SIZE_XZ, 0, j * CHUNK_SIZE_XZ + CHUNK_SIZE_XZ));
					see = see || PointViewable(glm::vec3(i * CHUNK_SIZE_XZ + CHUNK_SIZE_XZ, 0, j * CHUNK_SIZE_XZ + CHUNK_SIZE_XZ));
					see = see || PointViewable(glm::vec3(i * CHUNK_SIZE_XZ, CHUNK_SIZE_Y / 2, j * CHUNK_SIZE_XZ));

					if(see) {
						viewable.push_back(chunk->second);
					}
				}

			} else {

				Chunk* c = new Chunk(btWorld);
				c->pos.x = i;
				c->pos.z = j;
				chunks.insert({c->pos, c});

				pool.enqueue([c]() -> void {

					c->Generate();
					c->Build();
					c->generating = false;
				});
			}
		}
	}
}

void World::Simulate(double dT) {

	PlayerMovement(dT);
	btWorld->stepSimulation(dT, 100);
}

glm::vec3 World::CheckDirectionClear(glm::vec3 targetDirection, glm::vec3 pos, Chunk * chunk, bool positive) {
	glm::vec3 direction = targetDirection;
	glm::vec3 movementDirection;

	if(positive) {
		movementDirection = direction + pos;
	}else {
		movementDirection = pos - direction;
	}

	if(!chunk->Occupied(movementDirection.x, movementDirection.y - 1, movementDirection.z) &&
		!chunk->Occupied(movementDirection.x, movementDirection.y, movementDirection.z) &&
		!chunk->Occupied(movementDirection.x, movementDirection.y + 1, movementDirection.z)) {
			return targetDirection;
		}

	return glm::vec3(0, 0, 0);
}

void World::PlayerMovement(double dT) {
	Chunk::position camChunk = GetCameraChunk();
	auto chunk = chunks.find(camChunk);

	float x_position = cam->pos.x - (camChunk.x * CHUNK_SIZE_XZ);
	float z_position = cam->pos.z - (camChunk.z * CHUNK_SIZE_XZ);

	glm::vec3 cam_position = glm::vec3(x_position, cam->pos.y, z_position);

	if(chunk->second->Occupied(cam_position.x, cam->pos.y - 2, cam_position.z)) {
		cam->grounded = true;
	} else {
		cam->grounded = false;
	}

	static const unsigned char* keys = SDL_GetKeyboardState(NULL);

	cam->last_update += dT;

  glm::vec3 target = glm::vec3(0, 0, 0);

  if(!cam->flying) {
    float y_velocity = cam->btBody->getLinearVelocity().getY();
    if(y_velocity > 0) {
      target.y = cam->btBody->getLinearVelocity().getY();
    }
  }

  if (keys[SDL_SCANCODE_F]) {
		//cam->pos += cam->front * cam->speed * dT;
    if(cam->f_released) {
      cam->f_released = false;
      if(cam->flying) {
        cam->flying = false;
        cam->currentGravityVelocity = 0.0f;
      }else {
        cam->flying = true;
        cam->currentGravityVelocity = 0.0f;
        cam->btBody->setLinearVelocity(btVector3(0, 0, 0));
      }
    }
	}else {
    cam->f_released = true;
  }

	if (keys[SDL_SCANCODE_W]) {
		glm::vec3 player_forward = glm::normalize(glm::vec3(cam->front.x, 0, cam->front.z));
		if(cam->flying) {
			glm::vec3 newDirection = CheckDirectionClear(cam->front, cam_position, chunk->second, true);
			target += newDirection * cam->speed;
		} else {
			glm::vec3 newDirection = CheckDirectionClear(player_forward, cam_position, chunk->second, true);
			target += newDirection * cam->speed;
		}
	}

	if (keys[SDL_SCANCODE_S]) {
		glm::vec3 player_forward = glm::normalize(glm::vec3(cam->front.x, 0, cam->front.z));
		if(cam->flying) {
			glm::vec3 newDirection = CheckDirectionClear(cam->front, cam_position, chunk->second, false);
			target -= newDirection * cam->speed;
		} else {
			glm::vec3 newDirection = CheckDirectionClear(player_forward, cam_position, chunk->second, false);
			target -= newDirection * cam->speed;
		}
	}

	if (keys[SDL_SCANCODE_A]) {
		glm::vec3 player_right = glm::normalize(glm::vec3(cam->right.x, 0, cam->right.z));
		if(cam->flying) {
			glm::vec3 newDirection = CheckDirectionClear(cam->right, cam_position, chunk->second, false);
			target -= newDirection * cam->speed;
		} else {
			glm::vec3 newDirection = CheckDirectionClear(player_right, cam_position, chunk->second, false);
			target -= newDirection * cam->speed;
		}
	}

	if (keys[SDL_SCANCODE_D]) {
		glm::vec3 player_right = glm::normalize(glm::vec3(cam->right.x, 0, cam->right.z));
		if(cam->flying) {
			glm::vec3 newDirection = CheckDirectionClear(cam->right, cam_position, chunk->second, true);
			target += newDirection * cam->speed;
		} else {
			glm::vec3 newDirection = CheckDirectionClear(player_right, cam_position, chunk->second, true);
			target += newDirection * cam->speed;
		}
	}

	glm::vec3 player_up = cam_position.y + glm::normalize(glm::vec3(0, cam->up.y, 0));

	if (keys[SDL_SCANCODE_SPACE]) {
		//cam->pos += cam->right * cam->speed * dT;
		if(!chunk->second->Occupied(player_up.x, player_up.y, player_up.z)) {
	    if(!cam->flying && cam->grounded) {
	      //std::cout << "Jump" << std::endl;
	      target.y = 10;
	    }else if(cam->flying) {
	      target.y += cam->speed;
	    }
		}
	}

	if(chunk->second->Occupied(cam_position.x, player_up.y, cam_position.z)) {
		if(target.y > 0) {
			target.y = 0;
		}
	}

	if (keys[SDL_SCANCODE_RSHIFT] || keys[SDL_SCANCODE_LSHIFT]) {

		if(!cam->grounded) {
			if(cam->flying) {
				target.y -= cam->speed;
			}
		}
	}

	if(!cam->flying) {
		if(!cam->grounded) {
			if(cam->currentGravityVelocity == 0.0) {
				cam->currentGravityVelocity = -0.01;
			}

			cam->currentGravityVelocity -= (9.8/60);
			target.y += cam->currentGravityVelocity;
		} else {
			cam->currentGravityVelocity = 0.0;
		}
	}

  if(!cam->flying) {
    if(!cam->grounded) {
      if(cam->currentGravityVelocity == 0.0) {
        cam->currentGravityVelocity = -0.01;
      }
      cam->currentGravityVelocity -= (9.8/60);
      //std::cout << "Gravity: " << cam->currentGravityVelocity << std::endl;
      target.y += cam->currentGravityVelocity;
    }else {
      cam->currentGravityVelocity = 0.0;
      if(target.y < 0) {
        target.y = 0;
      }
    }
  }

  cam->btBody->setLinearVelocity(btVector3(target.x, target.y, target.z));
}

void World::Render(ShaderInfo info) {

	info.shader->Enable();
	info.ambient_light = ambient_light;
	info.wireframe = draw_wireframe;
	info.diffuse_light = diffuse_light;
	info.specular_light = specular_light;
	info.const_atten = const_atten;
	info.lin_atten = lin_atten;
	info.quad_atten = quad_atten;
	info.spot_dir = spot_dir;
	info.spot_cutoff = spot_cutoff;
	info.spot_exp = spot_exp;

	GetViewable();

	for(Chunk* c : viewable) {

		std::vector<Chunk::light> near_lights = GetLights(c);
		c->Render(info, near_lights);
		if(c->pos == GetCameraChunk()) {
			if(!c->hasPhysics) {
				c->SetPhysics();
			}
		}else {
			//c->DeletePhysics();
		}
	}
}

void World::RenderPlayer(ShaderInfo info) {

	cam->RenderPlayer(&info);
}
