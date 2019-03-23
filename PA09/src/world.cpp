
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

World::World() {

}

World::~World() {

	for(Object* o : objects) {
		if(Collider* c = dynamic_cast<Collider*>(o)) {
			btWorld->removeRigidBody(c->btBody);
		}
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

	for(Object* o : objects) {
		delete o;
	}
}

void World::Reset() {

	for(Object* o : objects) {
		o->Reset();
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

		Collider* selectedObject = dynamic_cast<Collider*>(objects[selected]);

		btTransform transform;
		selectedObject->btMotionState->getWorldTransform(transform);

		btVector3 world_pos = selectedObject->btBody->getCenterOfMassPosition();
		btVector3 world_origin = transform.getOrigin();
		btVector3 mass_pos = world_pos - world_origin;

		if (keys[SDL_SCANCODE_W]) {
			btVector3 force = btVector3(-10, 0, 0);
			selectedObject->btBody->applyForce(force, mass_pos);
		}
		if (keys[SDL_SCANCODE_S]) {
			btVector3 force = btVector3(10, 0, 0);
			selectedObject->btBody->applyForce(force, mass_pos);
		}
		if (keys[SDL_SCANCODE_A]) {
			btVector3 force = btVector3(0, 0, 10);
			selectedObject->btBody->applyForce(force, mass_pos);
		}
		if (keys[SDL_SCANCODE_D]) {
			btVector3 force = btVector3(0, 0, -10);
			selectedObject->btBody->applyForce(force, mass_pos);
		}

	}

	for(Object* o : objects) {

		Collider* c = dynamic_cast<Collider*>(o);
		Renderable* r = dynamic_cast<Renderable*>(o);
		if(c && r) {

			btTransform transform;
			btScalar mat[16];

			c->btMotionState->getWorldTransform(transform);
			transform.getOpenGLMatrix(mat);
			r->modelmx = glm::make_mat4(mat);

			btQuaternion q = transform.getRotation();
			glm::quat quat(q.w(),q.x(),q.y(),q.z());
			r->rotmx = glm::mat4(quat);

			if(o->name == "Sphere") {
				glm::vec3 position = glm::vec3(r->modelmx[3]);
				position.y = spotlight->position.y;
				spotlight->position = glm::vec4(position, 1);
			}
		}
	}
}

std::ostream& operator<<(std::ostream& out, glm::vec3 vec) {
	return out << vec.x << " " << vec.y << " " << vec.z;
}

void World::Render(ShaderInfo info) {

	// locations
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
	for(Object* o : objects) {
		if(num_lights >= 16) break;

		if(Light* p = dynamic_cast<Light*>(o)) {

			std::string light = "lights[" + std::to_string(num_lights) + "].";

			glUniform4fv(info.shader->GetUniformLocation((light + "pos").c_str()), 1, glm::value_ptr(p->position));
			glUniform3fv(info.shader->GetUniformLocation((light + "diffuse_color").c_str()), 1, glm::value_ptr(p->diffuse_color));
			glUniform3fv(info.shader->GetUniformLocation((light + "specular_color").c_str()), 1, glm::value_ptr(p->specular_color));
			glUniform1f(info.shader->GetUniformLocation((light + "constant_attenuation").c_str()), p->constant_atten);
			glUniform1f(info.shader->GetUniformLocation((light + "linear_attenuation").c_str()), p->linear_atten);
			glUniform1f(info.shader->GetUniformLocation((light + "quadratic_attenuation").c_str()), p->quad_atten);
			glUniform3fv(info.shader->GetUniformLocation((light + "spotlight_direction").c_str()), 1, glm::value_ptr(p->spotlight_dir));
			glUniform1f(info.shader->GetUniformLocation((light + "spotlight_cutoff").c_str()), p->spotlight_cutoff);
			glUniform1f(info.shader->GetUniformLocation((light + "spotlight_exponent").c_str()), p->spotlight_exp);

			num_lights++;
		}
	}
	glUniform1i(num_lights_loc, num_lights);

	for(Object* o : objects) {

		if(Renderable* r = dynamic_cast<Renderable*>(o)) {

			static float selected_ambient[] = {0.3f, 0.3f, 0.3f};

			if(selected != -1 && o == objects[selected]) {
				glUniform3fv(ambient_color_loc, 1, selected_ambient);
			} else {
				glUniform3fv(ambient_color_loc, 1, glm::value_ptr(info.default_ambient));
			}

			glUniform3fv(obj_ambient_loc, 1, glm::value_ptr(r->ambient));
			glUniform3fv(obj_diffuse_loc, 1, glm::value_ptr(r->diffuse));
			glUniform3fv(obj_specular_loc, 1, glm::value_ptr(r->specular));
			glUniform1f(obj_shine_loc, r->shine);
			glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(r->modelmx));

			r->s.Render();
		}
	}
}

void World::UI() {

	int ui_idx = 1;
	std::vector<const char*> available({"None"});

	for(unsigned int i = 0; i < objects.size(); i++) {
		if((int)i == selected) {
			ui_selected = ui_idx;
		}
		if(Collider* c = dynamic_cast<Collider*>(objects[i])) {
			if(c->mass > 0) {
				ui_idx++;
				available.push_back(objects[i]->name.c_str());
			}
		}
	}

	if(ImGui::CollapsingHeader("Object Properties")) {
		int idx = 0;
		ImGui::Indent();
		for(Object* o : objects) {
			ImGui::PushID(idx);
			idx++;

			if(ImGui::CollapsingHeader(o->name.c_str())) {
				ImGui::Indent();

				if(Renderable* r = dynamic_cast<Renderable*>(o)) {

					ImGui::Text("Model: %s", r->model.c_str());
					ImGui::Text("Texture: %s", r->texture.c_str());
					if(ImGui::CollapsingHeader("Ambient")) {
						ImGui::PushID(0);
						ImGui::Indent();
						ImGui::SliderFloat("R", &r->ambient.x, 0.0f, 1.0f);
						ImGui::SliderFloat("G", &r->ambient.y, 0.0f, 1.0f);
						ImGui::SliderFloat("B", &r->ambient.z, 0.0f, 1.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
					if(ImGui::CollapsingHeader("Diffuse")) {
						ImGui::PushID(1);
						ImGui::Indent();
						ImGui::SliderFloat("R", &r->diffuse.x, 0.0f, 1.0f);
						ImGui::SliderFloat("G", &r->diffuse.y, 0.0f, 1.0f);
						ImGui::SliderFloat("B", &r->diffuse.z, 0.0f, 1.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
					if(ImGui::CollapsingHeader("Specular")) {
						ImGui::PushID(2);
						ImGui::Indent();
						ImGui::SliderFloat("R", &r->specular.x, 0.0f, 1.0f);
						ImGui::SliderFloat("G", &r->specular.y, 0.0f, 1.0f);
						ImGui::SliderFloat("B", &r->specular.z, 0.0f, 1.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
					ImGui::SliderFloat("Shine", &r->shine, 0.1f, 20.0f);
				}

				if(Light* l = dynamic_cast<Light*>(o)) {
					if(ImGui::CollapsingHeader("Position")) {
						ImGui::PushID(0);
						ImGui::Indent();
						ImGui::SliderFloat("X", &l->position.x, -10.0f, 10.0f);
						ImGui::SliderFloat("Y", &l->position.y, -10.0f, 10.0f);
						ImGui::SliderFloat("Z", &l->position.z, -10.0f, 10.0f);
						ImGui::SliderFloat("W", &l->position.w, 0.0f, 1.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
					if(ImGui::CollapsingHeader("Diffuse Color")) {
						ImGui::PushID(1);
						ImGui::Indent();
						ImGui::SliderFloat("R", &l->diffuse_color.x, 0.0f, 1.0f);
						ImGui::SliderFloat("G", &l->diffuse_color.y, 0.0f, 1.0f);
						ImGui::SliderFloat("B", &l->diffuse_color.z, 0.0f, 1.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
					if(ImGui::CollapsingHeader("Specular Color")) {
						ImGui::PushID(2);
						ImGui::Indent();
						ImGui::SliderFloat("R", &l->specular_color.x, 0.0f, 1.0f);
						ImGui::SliderFloat("G", &l->specular_color.y, 0.0f, 1.0f);
						ImGui::SliderFloat("B", &l->specular_color.z, 0.0f, 1.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
					if(ImGui::CollapsingHeader("Attenuation")) {
						ImGui::PushID(3);
						ImGui::Indent();
						ImGui::SliderFloat("Constant", &l->constant_atten, 0.0f, 1.0f);
						ImGui::SliderFloat("Linear", &l->linear_atten, 0.0f, 1.0f);
						ImGui::SliderFloat("Quadratic", &l->quad_atten, 0.0f, 1.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
					if(ImGui::CollapsingHeader("Spotlight")) {
						ImGui::PushID(4);
						ImGui::Indent();
						if(ImGui::CollapsingHeader("Dierction")) {
							ImGui::Indent();
							ImGui::SliderFloat("X", &l->spotlight_dir.x, -1.0f, 1.0f);
							ImGui::SliderFloat("Y", &l->spotlight_dir.y, -1.0f, 1.0f);
							ImGui::SliderFloat("Z", &l->spotlight_dir.z, -1.0f, 1.0f);
							ImGui::Unindent();
						}
						ImGui::SliderFloat("Cutoff", &l->spotlight_cutoff, 0.0f, 180.0f);
						ImGui::SliderFloat("Exponent", &l->spotlight_exp, 0.0f, 10.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
				}

				ImGui::Unindent();
			}
			ImGui::PopID();
		}
		ImGui::Unindent();
	}

	ImGui::Separator();
	ImGui::Text("Physics");
	ImGui::Combo("Selected", &ui_selected, available.data(), available.size());

	if(!ui_selected) selected = -1;

	for(unsigned int i = 0; i < objects.size(); i++) {
		if(objects[i]->name == available[ui_selected]) {
			selected = i;
		}
	}
}

void World::NextSelected() {

	for(int i = (selected + 1) % objects.size(); i != selected; ++i %= objects.size()) {
		if(Collider* c = dynamic_cast<Collider*>(objects[i])) {
			if(c->mass > 0) {
				selected = i;
				break;
			}
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

	std::string contents;
	std::ifstream fin(path);
	getline(fin, contents, '\0');

	Object* o = Object::LoadJSON(contents);
	if(!o) {
		printf("Failed to load object from JSON\n");
		return false;
	}

	if(!o->Setup()) {
		return false;
	}

	if(Collider* c = dynamic_cast<Collider*>(o)) {
		btWorld->addRigidBody(c->btBody);
	}
	objects.push_back(o);

<<<<<<< HEAD
  if(o->name == "Spot Light") {
    Light * l = dynamic_cast<Light*>(o);
    spotlight = l;
  }
=======
	if(o->name == "Spot Light") {
		Light * l = dynamic_cast<Light*>(o);
		spotlight = l;
	}
>>>>>>> 235e2e86dbe6c169f55070955ad7dafbd12b2716
	return true;
}
