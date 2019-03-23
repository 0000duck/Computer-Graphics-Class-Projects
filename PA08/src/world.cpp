
#include "world.h"

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <imgui.h>
#include <SDL2/SDL.h>

bool isRegularFile(std::string path) {

    struct stat path_stat;
    stat(path.c_str(), &path_stat);
    return !!S_ISREG(path_stat.st_mode);
}

bool Object::LoadJSON(std::string json) {

	picojson::value v;
	std::string err = picojson::parse(v, json);

	if (!err.empty()) {
  		std::cerr << err << std::endl;
  		return false;
	}

	if (!v.is<picojson::object>()) {
		std::cerr << "JSON is not an object" << std::endl;
		return false;
	}

	const auto& obj = v.get<picojson::object>();
	return LoadJSONObj(obj);
}

#define stringfield(name) if(i.first == #name && i.second.is<std::string>()) name = i.second.get<std::string>();
#define floatfield(name) if(i.first == #name && i.second.is<double>()) name = i.second.get<double>();
#define vecfield(name) if(i.first == #name && i.second.is<picojson::array>()) { const auto& array = i.second.get<picojson::array>(); for(int i = 0; i < 3; i++) if(array[i].is<double>()) name[i] = array[i].get<double>();}
#define boolfield(name) if(i.first == #name && i.second.is<bool>()) name = i.second.get<bool>();

bool Object::LoadJSONObj(const picojson::object& obj) {

	shape = Shape::none;
	for(auto& i : obj) {

		if(i.first == "type" && i.second.is<std::string>()) {
			std::string type = i.second.get<std::string>();
			if(type == "plane") {
				shape = Shape::plane;
			} else if(type == "sphere") {
				shape = Shape::sphere;
			} else if(type == "box") {
				shape = Shape::box;
			} else if(type == "cylinder") {
				shape = Shape::cylinder;
			} else if(type == "mesh") {
				shape = Shape::mesh;
			} else {
				return false;
			}
			break;
		}
	}
	for(auto& i : obj) {

		stringfield(name);
		stringfield(model);
		stringfield(texture);

		floatfield(mass);

		vecfield(position);
		vecfield(velocity);

		if(i.first == "shape" && shape == Shape::plane && i.second.is<picojson::array>()) {
			const auto& array = i.second.get<picojson::array>();
			for(int i = 0; i < 4; i++) {
				if(array[i].is<double>()) {
					plane[i] = array[i].get<double>();
				}
			}
		}
		if(i.first == "shape" && shape == Shape::sphere && i.second.is<double>()) {
			sphere = i.second.get<double>();
		}
		if(i.first == "shape" && shape == Shape::box && i.second.is<picojson::array>()) {

			const auto& array = i.second.get<picojson::array>();
			for(int i = 0; i < 3; i++) {
				if(array[i].is<double>()) {
					box[i] = array[i].get<double>();
				}
			}
		}

	    if(i.first == "shape" && shape == Shape::cylinder && i.second.is<picojson::array>()) {

				const auto& array = i.second.get<picojson::array>();
				for(int i = 0; i < 3; i++) {
					if(array[i].is<double>()) {
						box[i] = array[i].get<double>();
					}
				}
			}
		}

	return true;
}

bool Object::Setup() {

	switch(shape) {
	case Shape::plane: {
		btShape = new btStaticPlaneShape(btVector3(plane.x, plane.y, plane.z), plane.w);
	} break;
	case Shape::sphere: {
		btShape = new btSphereShape(sphere);
	} break;
	case Shape::box: {
		btShape = new btBoxShape(btVector3(box.x, box.y, box.z));
	} break;
	case Shape::cylinder: {
		btShape = new btCylinderShape(btVector3(box.x, box.y, box.z));
	} break;
	case Shape::mesh: {
		btMesh = new btTriangleMesh();
	} break;
	default: break;
	}

	if(!s.LoadModel(model)) {
		return false;
	}
	if(!s.LoadTexture(texture)) {
		return false;
	}

	if(shape == Shape::mesh) {
		Scene::Mesh& mesh = s.getMesh();

		for(unsigned int i = 0; i < mesh.indices.size(); i += 3) {

			glm::vec3 pos1 = mesh.vertices[mesh.indices[i]].pos;
			glm::vec3 pos2 = mesh.vertices[mesh.indices[i+1]].pos;
			glm::vec3 pos3 = mesh.vertices[mesh.indices[i+2]].pos;

			btMesh->addTriangle(btVector3(pos1.x,pos1.y,pos1.z),btVector3(pos2.x,pos2.y,pos2.z),btVector3(pos3.x,pos3.y,pos3.z));
		}

		btShape = new btBvhTriangleMeshShape(btMesh, true);
	}

	btMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(position.x, position.y, position.z)));

	btInertia = btVector3(0,0,0);
	btShape->calculateLocalInertia(mass, btInertia);

	btRigidBody::btRigidBodyConstructionInfo btBodyCTI(mass, btMotionState, btShape, btInertia);
	btBody = new btRigidBody(btBodyCTI);

	if(mass > 0) {
		btBody->setActivationState(DISABLE_DEACTIVATION);
	}

	btBody->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));

	return true;
}

void Object::Reset() {

	btInertia = btVector3(0,0,0);

	btMotionState->setWorldTransform(btTransform(btQuaternion(0,0,0,1), btVector3(position.x, position.y, position.z)));
	btBody->setMotionState(btMotionState);

	btBody->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
	btBody->setAngularVelocity(btVector3(0,0,0));

	btBody->updateInertiaTensor();
}

void Object::Shutdown() {

	if(btBody) delete btBody;
	if(btMotionState) delete btMotionState;
	if(btShape) delete btShape;
	if(btMesh) delete btMesh;
	btShape = nullptr;
	btMesh = nullptr;
	btMotionState = nullptr;
	btBody = nullptr;

	s.DeleteMesh();
	s.DeleteTexture();
}

World::World() {

}

World::~World() {

	for(Object& o : objects) {
		btWorld->removeRigidBody(o.btBody);
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

	for(Object& o : objects) {
		o.Shutdown();
	}
}

void World::Reset() {

	for(Object& o : objects) {
		o.Reset();
	}
}

bool World::Initialize() {

	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

	solver = new btSequentialImpulseConstraintSolver();

	btWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	btWorld->setGravity(btVector3(5, -5, 0));

	return true;
}

void World::Update(unsigned int dT) {

	btWorld->stepSimulation(dT / 1000.0f);

	static const unsigned char* keys = SDL_GetKeyboardState(NULL);

	if(selected > -1) {

		Object& selectedObject = objects[selected];

		btTransform transform;
		selectedObject.btMotionState->getWorldTransform(transform);

		btVector3 world_pos = selectedObject.btBody->getCenterOfMassPosition();
		btVector3 world_origin = transform.getOrigin();
		btVector3 mass_pos = world_pos - world_origin;

		if (keys[SDL_SCANCODE_W]) {
			btVector3 force = btVector3(-10, 0, 0);
			selectedObject.btBody->applyForce(force, mass_pos);
		}
		if (keys[SDL_SCANCODE_S]) {
			btVector3 force = btVector3(10, 0, 0);
			selectedObject.btBody->applyForce(force, mass_pos);
		}
		if (keys[SDL_SCANCODE_A]) {
			btVector3 force = btVector3(0, 0, 10);
			selectedObject.btBody->applyForce(force, mass_pos);
		}
		if (keys[SDL_SCANCODE_D]) {
			btVector3 force = btVector3(0, 0, -10);
			selectedObject.btBody->applyForce(force, mass_pos);
		}
	}

	for(Object& o : objects) {
		btTransform transform;
		btScalar mat[16];

		o.btMotionState->getWorldTransform(transform);
		transform.getOpenGLMatrix(mat);
		o.modelmx = glm::make_mat4(mat);

		btQuaternion q = transform.getRotation();
		glm::quat quat(q.w(),q.x(),q.y(),q.z());
		o.rotmx = glm::mat4(quat);
	}
}

void World::Render(UniformLocs uniforms) {

	for(Object& o : objects) {

		if(selected != -1 && &o == &objects[selected])
			glUniform1f(uniforms.ambient, 0.75f);
		else
			glUniform1f(uniforms.ambient, 0.25f);
		glUniformMatrix4fv(uniforms.model, 1, GL_FALSE, glm::value_ptr(o.modelmx));
		glUniformMatrix4fv(uniforms.rotate, 1, GL_FALSE, glm::value_ptr(o.rotmx));

		o.s.Render();
	}
}

void World::UI() {

	int ui_idx = 1;
	std::vector<const char*> available({"None"});

	for(unsigned int i = 0; i < objects.size(); i++) {
		if((int)i == selected) {
			ui_selected = ui_idx;
		}
		if(objects[i].mass > 0) {
			ui_idx++;
			available.push_back(objects[i].name.c_str());
		}
	}

	ImGui::Text("Physics");
	ImGui::Combo("Selected", &ui_selected, available.data(), available.size());

	if(!ui_selected) selected = -1;

	for(unsigned int i = 0; i < objects.size(); i++) {
		if(objects[i].name == available[ui_selected]) {
			selected = i;
		}
	}
}

void World::NextSelected() {

	for(int i = (selected + 1) % objects.size(); i != selected; ++i %= objects.size()) {
		if(objects[i].mass > 0) {
			selected = i;
			break;
		}
	}
}

bool World::LoadObjects(std::string dir) {

	DIR *directory;
	dirent *entry;
	std::string dirPath = dir;

	if (dirPath.back() != '/' && dirPath.back() != '\\') {
		dirPath.append("/");
	}

	directory = opendir(dirPath.c_str());
	if (!directory) {
		std::cerr << "Failed to open directory at " << dirPath << std::endl;
		return false;
	}

	while((entry = readdir(directory))) {
		std::string entryName = entry->d_name;

		if(entryName != ".." && entryName != "." && isRegularFile(dirPath + entryName)) {

			std::cout << "Loading object "<< entryName << std::endl;
			if(!LoadObject(dirPath + entryName)) {
				std::cerr << "Failed to load object from file " << entryName << std::endl;
			}
		}
	}

	closedir(directory);

	return true;
}

bool World::LoadObject(std::string path) {

	Object o;

	std::string contents;
	std::ifstream fin(path);
	getline(fin, contents, '\0');

	if(!o.LoadJSON(contents)) {
    printf("Failed to Load JSON\n");
		return false;
	}

	if(!o.Setup()) {
		return false;
	}

	AddObject(o);
	return true;
}

void World::AddObject(const Object& o) {

	btWorld->addRigidBody(o.btBody);
	objects.push_back(o);
}
