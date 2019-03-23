
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

	// set up parameters from a json string
	static Object* LoadJSON(std::string json);

	virtual ~Object();

	// load paramters from a json object
	virtual void LoadJSONObj(const picojson::object& obj);
	// reset to default parameters
	virtual void Reset() = 0;
	// do any neccesary set up - e.g. bullet shapes
	virtual bool Setup() = 0;

	friend class World;
	friend void CheckCollisions(btDynamicsWorld *btWorld, btScalar timeStep);
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

	float restitution = 0.0;

	// load paramters from a json object
	virtual void LoadJSONObj(const picojson::object& obj);
	// set up bullet shape, motion, and body
	virtual bool Setup();
	// reset position & velocity
	virtual void Reset();

	friend class World;
	friend void CheckCollisions(btDynamicsWorld *btWorld, btScalar timeStep);
};

class Renderable : public virtual Object {
protected:
	virtual ~Renderable();

	glm::vec3 ambient, diffuse, specular, diffuse_boost;
	float shine = 1.0f, scale = 1.0f;
	int boost_cooldown = 0;
	
	std::string name, model, texture;
	int model_idx = 0;
	Scene s;
	glm::mat4 modelmx, rotmx;

	// load paramters from a json object
	virtual void LoadJSONObj(const picojson::object& obj);
	// set up scene, model, texture for rendering
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

	// load paramters from a json object
	virtual void LoadJSONObj(const picojson::object& obj);
	// useless
	virtual bool Setup();
	// reset to default parameters
	virtual void Reset();

	friend class World;
};

class Plane : public Collider, public Renderable {
protected:
	virtual ~Plane();

	glm::vec4 plane;

	// load paramters from a json object
	virtual void LoadJSONObj(const picojson::object& obj);
	virtual bool Setup();

	friend class World;
};

class Sphere : public Collider, public Renderable {
protected:
	virtual ~Sphere();

	float sphere = 0.0f;

	// load paramters from a json object
	virtual void LoadJSONObj(const picojson::object& obj);
	// set up collider/renderable info
	virtual bool Setup();

	friend class World;
};

class Cylinder : public Collider, public Renderable {
protected:
	virtual ~Cylinder();

	glm::vec3 cylinder;

	// load paramters from a json object
	virtual void LoadJSONObj(const picojson::object& obj);
	// set up collider/renderable info
	virtual bool Setup();

	friend class World;
};

class Box : public Collider, public Renderable {
protected:
	virtual ~Box();

	glm::vec3 box;

	// load paramters from a json object
	virtual void LoadJSONObj(const picojson::object& obj);
	// set up collider/renderable info
	virtual bool Setup();

	friend class World;
};

class Mesh : public Collider, public Renderable {
protected:
	virtual ~Mesh();

	btTriangleMesh* btMesh = nullptr;

	// load paramters from a json object
	virtual void LoadJSONObj(const picojson::object& obj);
	// set up collider/renderable info
	virtual bool Setup();

	friend class World;
};

#endif
