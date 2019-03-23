
#ifndef WORLD_H
#define WORLD_H

#include "graphics.h"
#include "scene.h"
#include "object.h"
#include "text.h"
#include "sound.h"
#include <SDL2/SDL.h>

class World {
public:
	World();
	~World();

	// set up bullet world
	bool Initialize(Sound* sound);
	// simulate over dT
	void Update(unsigned int dT);
	// render objects
	void Render(ShaderInfo info);

	// load objects from directory
	bool LoadObjects(std::string directory);
	// load single object
	bool LoadObject(std::string path);

	// reset all objects
	void Reset();
	// process keyboard events
	void KeyboardEvts(SDL_Event e);

	// display UI options
	void UI(Text* t);

private:

	// game logic info
	int score = 0, lives = 3, power = 0;
	bool playing = false, reset = true, gameover = false;

	// bullet info
	btDiscreteDynamicsWorld* btWorld = nullptr;
	btBroadphaseInterface* broadphase = nullptr;
	btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
	btCollisionDispatcher* dispatcher = nullptr;
	btSequentialImpulseConstraintSolver* solver = nullptr;

	// object store
	std::vector<Object*> objects;
	
	// specific game objects
	Collider *leftFlipper = nullptr, *rightFlipper = nullptr;
	Renderable* ball_r = nullptr;
	Collider*   ball_c = nullptr;
	Light* spotlight = nullptr;
	btHingeConstraint *leftHinge = nullptr, *rightHinge = nullptr;

	// process collisions for game logic
	void CheckCollisions(unsigned int dT);
	Sound* m_sound = nullptr;
};

#endif // WORLD_H
