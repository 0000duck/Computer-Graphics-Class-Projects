#ifndef CAMERA_H
#define CAMERA_H

#include "graphics_headers.h"
#include <btBulletDynamicsCommon.h>
#include "scene.h"

struct ShaderInfo;

class FreeCamera {

public:
	FreeCamera();
	~FreeCamera();

	glm::mat4 GetProjection(float w, float h);
	glm::mat4 GetView();
	glm::mat4 GetViewWithoutTranslate();
	void reset();
	void update();
	void move(int dx, int dy);
	void setDistance(float d) {}
	bool LoadModel();
	void RenderPlayer(ShaderInfo* info);

	bool third_person = false;

private:
	const float sens = 0.5f;

	glm::vec3 pos, front, up, right;
	float pitch, yaw, speed, fov;

	float last_update;
	float currentGravityVelocity = 0.0f;
	bool lock = false;
	bool flying = false;
	bool grounded = false;
	bool f_released = true;

	Scene scene;
	glm::mat4 modelmx, rotmx;

	btCollisionShape* btShape = nullptr;
	btDefaultMotionState* btMotionState = nullptr;
	btScalar mass = 1;
	btVector3 btInertia = btVector3(0,0,0);
	btRigidBody* btBody = nullptr;

	friend class Graphics;
	friend class World;
};

#endif /* CAMERA_H */
