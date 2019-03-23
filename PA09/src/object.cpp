
#include "object.h"

#define stringfield(name) if(i.first == #name && i.second.is<std::string>()) name = i.second.get<std::string>();
#define floatfield(name) if(i.first == #name && i.second.is<double>()) name = i.second.get<double>();
#define vec3field(name) if(i.first == #name && i.second.is<picojson::array>()) { const auto& array = i.second.get<picojson::array>(); for(int i = 0; i < 3; i++) if(array[i].is<double>()) name[i] = array[i].get<double>();}
#define vec4field(name) if(i.first == #name && i.second.is<picojson::array>()) { const auto& array = i.second.get<picojson::array>(); for(int i = 0; i < 4; i++) if(array[i].is<double>()) name[i] = array[i].get<double>();}
#define boolfield(name) if(i.first == #name && i.second.is<bool>()) name = i.second.get<bool>();

Object* Object::LoadJSON(std::string json) {

	picojson::value v;
	std::string err = picojson::parse(v, json);

	if (!err.empty()) {
  		std::cerr << err << std::endl;
  		return nullptr;
	}

	if (!v.is<picojson::object>()) {
		std::cerr << "JSON is not an object" << std::endl;
		return nullptr;
	}

	const auto& obj = v.get<picojson::object>();

	Object* ret = nullptr;
	for(auto& i : obj) {
		if(i.first == "type" && i.second.is<std::string>()) {
			std::string type = i.second.get<std::string>();
			if(type == "plane") {
				ret = new Plane;
			} else if(type == "sphere") {
				ret = new Sphere;
			} else if(type == "box") {
				ret = new Box;
			} else if(type == "cylinder") {
				ret = new Cylinder;
			} else if(type == "mesh") {
				ret = new Mesh;
			} else if(type == "light") {
				ret = new Light;
			}
		}
	}
	if(!ret) return ret;

	ret->LoadJSONObj(obj);
	ret->Setup();
	return ret;
}

void Object::LoadJSONObj(const picojson::object& obj) {

	for(auto& i : obj) {

		stringfield(name);
	}
}

void Collider::LoadJSONObj(const picojson::object& obj) {

	Object::LoadJSONObj(obj);

	for(auto& i : obj) {

		floatfield(mass);
		
		vec3field(position);
		vec3field(velocity);
		floatfield(restitution);
	}
}

void Renderable::LoadJSONObj(const picojson::object& obj) {

	Object::LoadJSONObj(obj);

	for(auto& i : obj) {

		stringfield(model);
		stringfield(texture);

		vec3field(specular);
		vec3field(diffuse);
		vec3field(ambient);

		floatfield(shine);
	}
}

void Light::LoadJSONObj(const picojson::object& obj) {

	Object::LoadJSONObj(obj);

	for(auto& i : obj) {

		vec4field(position);
		vec3field(diffuse_color);
		vec3field(specular_color);
		
		floatfield(constant_atten);
		floatfield(linear_atten);
		floatfield(quad_atten);

		vec3field(spotlight_dir);
		floatfield(spotlight_cutoff);
		floatfield(spotlight_exp);
	}
}

void Plane::LoadJSONObj(const picojson::object& obj) {

	Collider::LoadJSONObj(obj);
	Renderable::LoadJSONObj(obj);

	for(auto& i : obj) {
		if(i.first == "shape" && i.second.is<picojson::array>()) {

			const auto& array = i.second.get<picojson::array>();
			for(int i = 0; i < 4; i++) {
				if(array[i].is<double>()) {
					plane[i] = array[i].get<double>();
				}
			}
		}
	}
}

void Sphere::LoadJSONObj(const picojson::object& obj) {

	Collider::LoadJSONObj(obj);
	Renderable::LoadJSONObj(obj);

	for(auto& i : obj) {
		if(i.first == "shape" && i.second.is<double>()) {

			sphere = i.second.get<double>();
		}
	}
}

void Cylinder::LoadJSONObj(const picojson::object& obj) {

	Collider::LoadJSONObj(obj);
	Renderable::LoadJSONObj(obj);

	for(auto& i : obj) {
		if(i.first == "shape" && i.second.is<picojson::array>()) {

			const auto& array = i.second.get<picojson::array>();
			for(int i = 0; i < 3; i++) {
				if(array[i].is<double>()) {
					cylinder[i] = array[i].get<double>();
				}
			}
		}
	}
}

void Box::LoadJSONObj(const picojson::object& obj) {

	Collider::LoadJSONObj(obj);
	Renderable::LoadJSONObj(obj);

	for(auto& i : obj) {
		if(i.first == "shape" && i.second.is<picojson::array>()) {

			const auto& array = i.second.get<picojson::array>();
			for(int i = 0; i < 3; i++) {
				if(array[i].is<double>()) {
					box[i] = array[i].get<double>();
				}
			}
		}
	}
}

void Mesh::LoadJSONObj(const picojson::object& obj) {

	Collider::LoadJSONObj(obj);
	Renderable::LoadJSONObj(obj);
}

bool Collider::Setup() {

	btMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(position.x, position.y, position.z)));

	btInertia = btVector3(0,0,0);
	btShape->calculateLocalInertia(mass, btInertia);

	btRigidBody::btRigidBodyConstructionInfo btBodyCTI(mass, btMotionState, btShape, btInertia);
	btBody = new btRigidBody(btBodyCTI);

	if(mass > 0) {
		btBody->setActivationState(DISABLE_DEACTIVATION);
	}

	btBody->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
	btBody->setRestitution(restitution);

	return true;
}

bool Renderable::Setup() {

	if(!s.LoadModel(model)) {
		return false;
	}
	if(!s.LoadTexture(texture)) {
		return false;
	}

	return true;
}

bool Light::Setup() {

	return true;
}

bool Plane::Setup() {

	if(!Renderable::Setup()) return false;
	btShape = new btStaticPlaneShape(btVector3(plane.x, plane.y, plane.z), plane.w);
	if(!Collider::Setup()) return false;

	return true;
}

bool Sphere::Setup() {

	if(!Renderable::Setup()) return false;
	btShape = new btSphereShape(sphere);
	if(!Collider::Setup()) return false;

	return true;
}

bool Box::Setup() {

	if(!Renderable::Setup()) return false;
	btShape = new btBoxShape(btVector3(box.x, box.y, box.z));
	if(!Collider::Setup()) return false;

	return true;
}

bool Cylinder::Setup() {

	if(!Renderable::Setup()) return false;
	btShape = new btCylinderShape(btVector3(cylinder.x, cylinder.y, cylinder.z));
	if(!Collider::Setup()) return false;

	return true;
}

bool Mesh::Setup() {

	if(!Renderable::Setup()) return false;

	btMesh = new btTriangleMesh();

	Scene::Mesh& mesh = s.getMesh();

	for(unsigned int i = 0; i < mesh.indices.size(); i += 3) {

		glm::vec3 pos1 = mesh.vertices[mesh.indices[i]].pos;
		glm::vec3 pos2 = mesh.vertices[mesh.indices[i+1]].pos;
		glm::vec3 pos3 = mesh.vertices[mesh.indices[i+2]].pos;

		btMesh->addTriangle(btVector3(pos1.x,pos1.y,pos1.z),btVector3(pos2.x,pos2.y,pos2.z),btVector3(pos3.x,pos3.y,pos3.z));
	}

	btShape = new btBvhTriangleMeshShape(btMesh, true);

	if(!Collider::Setup()) return false;

	return true;
}

Object::~Object() {

}

Collider::~Collider() {

	if(btBody) delete btBody;
	if(btMotionState) delete btMotionState;
	if(btShape) delete btShape;
	btShape = nullptr;
	btMotionState = nullptr;
	btBody = nullptr;
}

Renderable::~Renderable() {

	s.DeleteMesh();
	s.DeleteTexture();
}

Light::~Light() {

}

Plane::~Plane() {

}

Sphere::~Sphere() {

}

Cylinder::~Cylinder() {

}

Box::~Box() {

}

Mesh::~Mesh() {

	if(btMesh) delete btMesh;
	btMesh = nullptr;
}

void Collider::Reset() {

	btInertia = btVector3(0,0,0);

	btMotionState->setWorldTransform(btTransform(btQuaternion(0,0,0,1), btVector3(position.x, position.y, position.z)));
	btBody->setMotionState(btMotionState);

	btBody->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
	btBody->setAngularVelocity(btVector3(0,0,0));

	btBody->updateInertiaTensor();
}

void Light::Reset() {

}
