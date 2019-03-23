
#include "world.h"

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <imgui.h>
#include <SDL2/SDL.h>
#include <map>
#include <sstream>

bool isRegularFile(std::string path) {

	struct stat path_stat;
	stat(path.c_str(), &path_stat);
	return !!S_ISREG(path_stat.st_mode);
}

void World::CheckCollisions(unsigned int dT) {
	
	std::map<btCollisionObject*, std::pair<Object*,Object*>> collisions;

	int manifolds = dispatcher->getNumManifolds();
	for (int i = 0; i < manifolds; i++) {
		btPersistentManifold* contact = dispatcher->getManifoldByIndexInternal(i);
		btCollisionObject* a = (btCollisionObject*)contact->getBody0();
		btCollisionObject* b = (btCollisionObject*)contact->getBody1();

		int contacts = contact->getNumContacts();
		for (int j = 0; j < contacts; j++) {
			btManifoldPoint& pt = contact->getContactPoint(j);
			
			if (pt.getDistance() < 0.8f) {

				if (collisions.find(a) == collisions.end()) {
					collisions[a] = {(Object*)a->getUserPointer(), (Object*)b->getUserPointer()};
				}
				if (collisions.find(b) == collisions.end()) {
					collisions[b] = {(Object*)a->getUserPointer(), (Object*)b->getUserPointer()};
				}
			}
		}
	}

	for(Object* o : objects) {
		if(Renderable* r = dynamic_cast<Renderable*>(o)) {
			if(r->boost_cooldown > 0) r->boost_cooldown -= dT;
			if(r->boost_cooldown <= 0) r->diffuse_boost = glm::vec3(0.0f);
		}
	}

	static Object* last = nullptr;

	for(auto collision : collisions) {
		Object *a = collision.second.first, *b = collision.second.second;

		if(a->name == "Ball" && b->name == "CylinderBumpers") {
			
			if(last != b) {
				score += 2;
				last = b;
				std::cout << "Score: " << score << std::endl;
				m_sound->Play("bounce");
			}

			if(Renderable* r = dynamic_cast<Renderable*>(b)) {
				r->diffuse_boost = glm::vec3(0.4f);
				r->boost_cooldown = 250;
			}
			break;
		}

		if(a->name == "Ball" && b->name == "Reset" && last != b) {
			ball_c->Reset();
			lives--;
			reset = true;
			last = b;
			if(lives > 0)
				m_sound->Play("hit_reset");
			break;
		}

		if(a->name == "Ball" && b->name == "Bumpers") {
			m_sound->Play("hit_bumper");
		}

		if(a->name == "Ball" && b->name != "Board" && b->name != "Ramp 2") {
			last = b;
		}
	}
}

World::World() {
}

World::~World() {

	for(Object* o : objects) {
		if(Collider* c = dynamic_cast<Collider*>(o)) {
			btWorld->removeRigidBody(c->btBody);
		}
	}

	if(leftHinge) delete leftHinge;
	if(rightHinge) delete rightHinge;
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
	leftHinge = nullptr;
	rightHinge = nullptr;

	for(Object* o : objects) {
		delete o;
	}
}

void World::Reset() {

	for(Object* o : objects) {
		o->Reset();
	}
}

bool World::Initialize(Sound* sound) {

	m_sound = sound;

	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

	solver = new btSequentialImpulseConstraintSolver();

	btWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	btWorld->setGravity(btVector3(10, -10, 0));

	return true;
}

void World::Update(unsigned int dT) {

	btWorld->stepSimulation(dT / 1000.0f, 300, 1.0/120.0);
	CheckCollisions(dT);

	static const unsigned char* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_LEFT]) {
		btTransform transform;
		leftFlipper->btMotionState->getWorldTransform(transform);
		btScalar yaw, pitch, roll;

		btMatrix3x3 mat = btMatrix3x3(transform.getRotation());
		mat.getEulerYPR(yaw, pitch, roll);

		if(2 * pitch + M_PI <= 4.7) {
			leftFlipper->btBody->setAngularVelocity(btVector3(0, 15, 0));
			m_sound->Play("activate_flipper");
		} else {
			leftFlipper->btBody->setAngularVelocity(btVector3(0, 0, 0));
		}
	} else {
		btTransform transform;
		leftFlipper->btMotionState->getWorldTransform(transform);

		btScalar yaw, pitch, roll;

		btMatrix3x3 mat = btMatrix3x3(transform.getRotation());
		mat.getEulerYPR(yaw, pitch, roll);

		if(2 * pitch + M_PI >= 3.2) {
			leftFlipper->btBody->setAngularVelocity(btVector3(0, -3, 0));
		} else {
			leftFlipper->btBody->setAngularVelocity(btVector3(0, 0, 0));
		}
	}

	if (keys[SDL_SCANCODE_RIGHT]) {
		btTransform transform;
		rightFlipper->btMotionState->getWorldTransform(transform);
		btScalar yaw, pitch, roll;

		btMatrix3x3 mat = btMatrix3x3(transform.getRotation());
		mat.getEulerYPR(yaw, pitch, roll);

		if(2 * pitch + M_PI >= 1.5) {
			rightFlipper->btBody->setAngularVelocity(btVector3(0, -15, 0));
			m_sound->Play("activate_flipper");
		} else {
			rightFlipper->btBody->setAngularVelocity(btVector3(0, 0, 0));
		}
	} else {
		btTransform transform;
		rightFlipper->btMotionState->getWorldTransform(transform);

		btScalar yaw, pitch, roll;

		btMatrix3x3 mat = btMatrix3x3(transform.getRotation());
		mat.getEulerYPR(yaw, pitch, roll);

		if(2 * pitch + M_PI <= 3) {
			rightFlipper->btBody->setAngularVelocity(btVector3(0, 3, 0));
		} else {
			rightFlipper->btBody->setAngularVelocity(btVector3(0, 0, 0));
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
		}
	}

	glm::vec3 position = glm::vec3(ball_r->modelmx[3]);
	position = glm::vec3(position.x, position.y + 5, position.z);
	spotlight->position = glm::vec4(position, 1);

	if(lives == 0) {
		playing = false;
		lives = 3;
		reset = true;
		gameover = true;
		m_sound->Play("game_over");
	}
}

std::ostream& operator<<(std::ostream& out, glm::vec3 vec) {
	return out << vec.x << " " << vec.y << " " << vec.z;
}

void World::KeyboardEvts(SDL_Event e) {

	if(e.type == SDL_KEYDOWN) {

		if(e.key.keysym.sym == SDLK_RETURN && reset) {
			reset = false;

			ball_c->btBody->applyCentralImpulse(btVector3(-27 - power * 3,0,0));
			if(!playing) {
				playing = true;
				gameover = false;
				score = 0;
			} 
			m_sound->Play("launch_ball");
		}

		if(e.key.keysym.sym == SDLK_UP) {
			power += 1;
			if(power > 10) power = 10;
		}
		if(e.key.keysym.sym == SDLK_DOWN) {
			power -= 1;
			if(power < 0) power = 0;
		}
	}
}

void World::UI(Text* t) {

	std::stringstream sscore, slives, spower;
	sscore << "Score: " << score;
	slives << "Lives: "  << lives;
	spower << power * 10 << "%";
	if(!gameover)
		t->AddText("P I N B A L L", {-26, 9, 3.5}, {0, 0, -1}, 20.0f);
	else 
		t->AddText("GAME OVER!", {-26, 9, 3.5}, {0, 0, -1}, 20.0f);
	t->AddText(sscore.str(), {-26, 7, 2}, {0, 0, -1}, 30.0f);
	t->AddText(spower.str(), {7, 4, -9.5}, {0, 0, -1}, 40.0f);

	if(playing && reset) {
		t->AddText(slives.str(), {-26, 5.5, 0.8}, {0, 0, -1}, 55.0f);
		t->AddText("enter to shoot", {-26, 4.5, 2}, {0, 0, -1}, 55.0f);
	} if(playing && !reset) {
		t->AddText(slives.str(), {-26, 5.5, 0.8}, {0, 0, -1}, 55.0f);
	} else if(!playing) {
		t->AddText("enter to shoot", {-26, 5.5, 2}, {0, 0, -1}, 55.0f);
		t->AddText("up/down to adjust power  |  left/right flippers", {-26, 4.5, 7.2}, {0, 0, -1}, 55.0f);
	}

	if(ImGui::CollapsingHeader("Object Properties")) {
		int idx = 0;
		ImGui::Indent();
		for(Object* o : objects) {
			ImGui::PushID(idx);
			idx++;

			if(ImGui::CollapsingHeader(o->name.c_str())) {
				ImGui::Indent();

				if(Collider* c = dynamic_cast<Collider*>(o)) {
					ImGui::SliderFloat("Mass", &c->mass, 0.0f, 5.0f);
					ImGui::SliderFloat("Restitution", &c->restitution, 0.0f, 1.0f);
					if(ImGui::Button("Reset")) {
						c->Reset();
					}
				}

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
						ImGui::PushID(3);
						ImGui::Indent();
						ImGui::SliderFloat("X", &l->position.x, -10.0f, 10.0f);
						ImGui::SliderFloat("Y", &l->position.y, -10.0f, 10.0f);
						ImGui::SliderFloat("Z", &l->position.z, -10.0f, 10.0f);
						ImGui::SliderFloat("W", &l->position.w, 0.0f, 1.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
					if(ImGui::CollapsingHeader("Diffuse Color")) {
						ImGui::PushID(4);
						ImGui::Indent();
						ImGui::SliderFloat("R", &l->diffuse_color.x, 0.0f, 1.0f);
						ImGui::SliderFloat("G", &l->diffuse_color.y, 0.0f, 1.0f);
						ImGui::SliderFloat("B", &l->diffuse_color.z, 0.0f, 1.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
					if(ImGui::CollapsingHeader("Specular Color")) {
						ImGui::PushID(5);
						ImGui::Indent();
						ImGui::SliderFloat("R", &l->specular_color.x, 0.0f, 1.0f);
						ImGui::SliderFloat("G", &l->specular_color.y, 0.0f, 1.0f);
						ImGui::SliderFloat("B", &l->specular_color.z, 0.0f, 1.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
					if(ImGui::CollapsingHeader("Attenuation")) {
						ImGui::PushID(6);
						ImGui::Indent();
						ImGui::SliderFloat("Constant", &l->constant_atten, 0.0f, 1.0f);
						ImGui::SliderFloat("Linear", &l->linear_atten, 0.0f, 1.0f);
						ImGui::SliderFloat("Quadratic", &l->quad_atten, 0.0f, 1.0f);
						ImGui::Unindent();
						ImGui::PopID();
					}
					if(ImGui::CollapsingHeader("Spotlight")) {
						ImGui::PushID(7);
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

			glm::mat4 scalemx = glm::scale(glm::mat4(1.0f), glm::vec3(r->scale));

			glUniform3fv(ambient_color_loc, 1, glm::value_ptr(info.default_ambient));
			glUniform3fv(obj_ambient_loc, 1, glm::value_ptr(r->ambient));
			glUniform3fv(obj_diffuse_loc, 1, glm::value_ptr(r->diffuse + r->diffuse_boost));
			glUniform3fv(obj_specular_loc, 1, glm::value_ptr(r->specular));
			glUniform1f(obj_shine_loc, r->shine);
			glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(r->modelmx * scalemx));

			r->s.Render();
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

	if(o->name == "Ball") {
		ball_r = dynamic_cast<Renderable*>(o);
		ball_c = dynamic_cast<Collider*>(o);
	}
	if(o->name == "Spotlight") {
		spotlight = dynamic_cast<Light*>(o);
	}

	if(Collider* c = dynamic_cast<Collider*>(o)) {
		btWorld->addRigidBody(c->btBody);

		if(o->name == "Left Flipper") {
			leftFlipper = c;
			leftHinge = new btHingeConstraint(*(c->btBody), btVector3(0,0,0), btVector3(0,1,0), true);
			btWorld->addConstraint(leftHinge);
		}
		if(o->name == "Right Flipper") {
			rightFlipper = c;
			rightHinge = new btHingeConstraint(*(c->btBody), btVector3(0,0,0), btVector3(0,1,0), true);
			btWorld->addConstraint(rightHinge);
		}
		if(o->name == "Reset") {
			c->btBody->setCollisionFlags(c->btBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
			// c->btBody->setCollisionFlags(c->btBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		}
		if(o->name == "CylinderBumpers") {
			// c->btBody->setCollisionFlags(c->btBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
			c->btBody->setCollisionFlags(c->btBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		}
 	}
	objects.push_back(o);

	return true;
}
