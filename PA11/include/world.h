
#ifndef CHUNK_H
#define CHUNK_H

#include "graphics_headers.h"
#include "graphics.h"
#include "camera.h"
#include "window.h"
#include <unordered_map>
#include <btBulletDynamicsCommon.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <ThreadPool.h>

#define CHUNK_SIZE_XZ 16
#define CHUNK_SIZE_Y  256

class World;

class Chunk {
private:
	struct block {
		uint8_t texture = 255; // 255 = air
	};

	struct position {
		int x = 0, z = 0;
		position(int x = 0, int z = 0);
		bool operator==(const Chunk::position& other) const;
	};

	struct vertex {
		glm::vec3 pos;
		glm::vec3 tex;
		glm::vec3 norm;
	};

	struct light {
		glm::vec4 pos;
	};

public:
	Chunk(btDiscreteDynamicsWorld * btWorld);
	~Chunk();

	void Render(ShaderInfo info, const std::vector<light>& near_lights);
	void Build();
	void Generate();
	bool Occupied(int x, int y, int z);
	void OpenGL();

	void AddQuad(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, size_t width, size_t height, block type, glm::vec3 normal);
	void AddLight(float x, float y, float z);
	void SetPhysics();
	void DeletePhysics();

private:
	block blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_XZ][CHUNK_SIZE_Y];
	position pos;

	std::mutex mesh_swap;
	std::vector<vertex> mesh;

	std::vector<light> lights;

	int buffered_quads = 0;
	GLuint VAO = 0, VBO = 0;
	bool ogl_refresh = false;
	std::atomic<bool> generating;

	btDiscreteDynamicsWorld * btWorld = nullptr;
	btCollisionShape* btShape = nullptr;
	btDefaultMotionState* btMotionState = nullptr;
	btScalar mass = 0;
	btVector3 btInertia = btVector3(0,0,0);
	btRigidBody* btBody = nullptr;
	bool hasPhysics = false;

	friend class World;
	friend struct std::hash<position>;
};

namespace std {
	template<>
	struct hash<Chunk::position> {
		uint64_t operator()(const Chunk::position& pos) const {
			hash<int> h;
			return h(pos.x) ^ h(pos.z);
		}
	};
}

class World {
public:
	World(FreeCamera* cam, int* w, int* h);
	~World();

	void StartGenerating();
	bool StopGenerating();

	void Render(ShaderInfo info);
	void UI();
	void Scroll(int y);
	void Simulate(double dT);
	void PlayerMovement(double dT);
	glm::vec3 CheckDirectionClear(glm::vec3 targetDirection, glm::vec3 pos, Chunk * chunk, bool positive);
	void AddLight();
	void TryDestroy();
	void TryPlace();
	void RenderPlayer(ShaderInfo info);

	void GetViewable();
	Chunk::position GetCameraChunk();

private:
	std::mutex world_mut;
	ThreadPool pool;

	glm::vec3 ambient_light = glm::vec3(0.25f), diffuse_light = glm::vec3(0.5f), specular_light = glm::vec3(0.5f);
	glm::vec3 spot_dir = glm::vec3(0, 1, 0);
	float quad_atten = 0.05f, lin_atten = 0.2f, const_atten = 0.2f, spot_cutoff = 90.0f, spot_exp = 1.0f;

	int *w = nullptr, *h = nullptr;
	int select = 0;
	bool draw_wireframe = false;
	GLuint textures = 0;
	FreeCamera* cam = nullptr;
	int view_distance = 8;
	std::vector<Chunk*> viewable;
	std::vector<GLuint> textures_as_a_list;

	void LoadTextures();
	bool LoadTexture(std::string file, int index);
	bool PointViewable(glm::vec3 point);
	std::vector<Chunk::light> GetLights(Chunk* c);

	std::unordered_map<Chunk::position, Chunk*> chunks;

	btDiscreteDynamicsWorld* btWorld = nullptr;
	btBroadphaseInterface* broadphase = nullptr;
	btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
	btCollisionDispatcher* dispatcher = nullptr;
	btSequentialImpulseConstraintSolver* solver = nullptr;
};

#endif // CHUNK_H
