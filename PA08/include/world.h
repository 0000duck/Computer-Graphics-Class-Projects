
#ifndef WORLD_H
#define WORLD_H

#include <btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>
#include <picojson.h>
#include "graphics.h"
#include "scene.h"

enum class Shape {
	none,
	plane,
	sphere,
	box,
	cylinder,
	mesh
};

class Object {
	// shape info
	Shape shape = Shape::none;
	btCollisionShape* btShape = nullptr;
	btTriangleMesh* btMesh = nullptr;

	// initial values
	glm::vec3 box, position, velocity;
	glm::vec4 plane;
	double sphere;

	// bullet info
	btDefaultMotionState* btMotionState = nullptr;
	btScalar mass;
	btVector3 btInertia;
	btRigidBody* btBody = nullptr;

	// rendering
	std::string name, model, texture;
	Scene s;
	glm::mat4 modelmx, rotmx;

	bool LoadJSON(std::string json);
	bool LoadJSONObj(const picojson::object& obj);

	bool Setup();
	void Shutdown();
	void Reset();

	friend class World;
};

class World {
public:
	World();
	~World();

	bool Initialize();
	void Update(unsigned int dT);
	void Render(UniformLocs uniforms);

	bool LoadObjects(std::string directory);
	bool LoadObject(std::string path);
	void AddObject(const Object& o);

	void Reset();
	void NextSelected();

	void UI();

private:

	btDiscreteDynamicsWorld* btWorld = nullptr;

	btBroadphaseInterface* broadphase = nullptr;
	btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
	btCollisionDispatcher* dispatcher = nullptr;
	btSequentialImpulseConstraintSolver* solver = nullptr;

	std::vector<Object> objects;
	int selected = -1, ui_selected = 0;
};

#endif // WORLD_H
