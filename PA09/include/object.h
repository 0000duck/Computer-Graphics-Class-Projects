
#ifndef OBJECT_H
#define OBJECT_H

#include <btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>
#include <picojson.h>
#include "scene.h"

class Object {
protected:

	std::string name;

	static Object* LoadJSON(std::string json);

	virtual ~Object();

	virtual void LoadJSONObj(const picojson::object& obj);
	virtual void Reset() = 0;
	virtual bool Setup() = 0;

	friend class World;
};

class Collider : public virtual Object {
protected:
	virtual ~Collider();

	btCollisionShape* btShape = nullptr;
	btDefaultMotionState* btMotionState = nullptr;
	btScalar mass = 0;
	btVector3 btInertia = btVector3(0,0,0);
	btRigidBody* btBody = nullptr;

	glm::vec3 position, velocity;

	double restitution = 0.0;

	virtual void LoadJSONObj(const picojson::object& obj);
	virtual bool Setup();
	virtual void Reset();

	friend class World;
};

class Renderable : public virtual Object {
protected:
	virtual ~Renderable();

	glm::vec3 ambient, diffuse, specular;
	float shine = 1.0f;
	
	std::string name, model, texture;
	Scene s;
	glm::mat4 modelmx, rotmx;

	virtual void LoadJSONObj(const picojson::object& obj);
	virtual bool Setup();

	friend class World;
};

class Light : public virtual Object {
protected:
	virtual ~Light();

	glm::vec4 position; // w = 0 for directional light
	glm::vec3 diffuse_color, specular_color;
	float constant_atten = 1.0f, linear_atten = 1.0f, quad_atten = 1.0f;

	glm::vec3 spotlight_dir;
	float spotlight_cutoff = 180.0f, spotlight_exp = 1.0f;

	virtual void LoadJSONObj(const picojson::object& obj);
	virtual bool Setup();
	virtual void Reset();

	friend class World;
};

class Plane : public Collider, public Renderable {
protected:
	virtual ~Plane();

	glm::vec4 plane;

	virtual void LoadJSONObj(const picojson::object& obj);
	virtual bool Setup();

	friend class World;
};

class Sphere : public Collider, public Renderable {
protected:
	virtual ~Sphere();

	float sphere = 0.0f;

	virtual void LoadJSONObj(const picojson::object& obj);
	virtual bool Setup();

	friend class World;
};

class Cylinder : public Collider, public Renderable {
protected:
	virtual ~Cylinder();

	glm::vec3 cylinder;

	virtual void LoadJSONObj(const picojson::object& obj);
	virtual bool Setup();

	friend class World;
};

class Box : public Collider, public Renderable {
protected:
	virtual ~Box();

	glm::vec3 box;

	virtual void LoadJSONObj(const picojson::object& obj);
	virtual bool Setup();

	friend class World;
};

class Mesh : public Collider, public Renderable {
protected:
	virtual ~Mesh();

	btTriangleMesh* btMesh = nullptr;

	virtual void LoadJSONObj(const picojson::object& obj);
	virtual bool Setup();

	friend class World;
};

#endif
