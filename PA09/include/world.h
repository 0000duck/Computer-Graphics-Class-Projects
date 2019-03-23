
#ifndef WORLD_H
#define WORLD_H

#include "graphics.h"
#include "scene.h"
#include "object.h"

class World {
public:
	World();
	~World();

	bool Initialize();
	void Update(unsigned int dT);
	void Render(ShaderInfo info);

	bool LoadObjects(std::string directory);
	bool LoadObject(std::string path);

	void Reset();
	void NextSelected();

	void UI();

private:

	btDiscreteDynamicsWorld* btWorld = nullptr;

	btBroadphaseInterface* broadphase = nullptr;
	btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
	btCollisionDispatcher* dispatcher = nullptr;
	btSequentialImpulseConstraintSolver* solver = nullptr;

	std::vector<Object*> objects;
	Light * spotlight;
	int selected = -1, ui_selected = 0;
};

#endif // WORLD_H
