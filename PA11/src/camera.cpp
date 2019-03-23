
#include "camera.h"
#include "graphics.h"

FreeCamera::FreeCamera() {
	reset();

	btShape = new btBoxShape(btVector3(1, 2, 1));
	btShape->calculateLocalInertia(mass, btInertia);
	btMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(pos.x, pos.y, pos.z)));

	btRigidBody::btRigidBodyConstructionInfo btBodyCTI(mass, btMotionState, btShape, btInertia);
	btBody = new btRigidBody(btBodyCTI);

	btBody->setActivationState(DISABLE_DEACTIVATION);
	btBody->setLinearVelocity(btVector3(0,0,0));
	btBody->setRestitution(0);

	update();
}

FreeCamera::~FreeCamera() {
}

glm::mat4 FreeCamera::GetProjection(float w, float h) {
	return glm::perspective(glm::radians(fov), w/h, 0.01f, 1000.0f);
}

glm::mat4 FreeCamera::GetView() {
	if(third_person)
		return glm::lookAt(pos - front, pos + front, up);
	else 
		return glm::lookAt(pos, pos + front, up);
}

glm::mat4 FreeCamera::GetViewWithoutTranslate() {
	if(third_person)
		return glm::lookAt(-front, front, up);
	else
		return glm::lookAt(glm::vec3(0.0f), front, up);
}

void FreeCamera::reset() {
	fov = 60.0f;
	pitch = -45.0f;
	yaw = 225.0f;
	speed = 5.0f;
	pos = glm::vec3(8, 125, 8);
}

void FreeCamera::RenderPlayer(ShaderInfo* info) {
	
	if(!third_person) return;
	
	glm::mat4 transform;

	glm::vec3 pt = pos;
	pt.y -= 1.5;

	transform = glm::translate(transform, pt);
	transform = glm::rotate(transform, glm::radians(-yaw), glm::vec3(0, 1, 0));
	transform = glm::scale(transform, glm::vec3(0.2f));

	glUniformMatrix4fv(info->shader->GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(transform));
	scene.Render();
}

void FreeCamera::update() {
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);
	right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
	up = glm::normalize(glm::cross(right, front));

	btTransform transform;
	btMotionState->getWorldTransform(transform);
	pos = glm::vec3(transform.getOrigin().getX(), transform.getOrigin().getY(), transform.getOrigin().getZ());
}

void FreeCamera::move(int dx, int dy) {
	const float sens = 0.1f;
	yaw += dx * sens;
	pitch -= dy * sens;
	if (yaw > 360.0f) yaw = 0.0f;
	else if (yaw < 0.0f) yaw = 360.0f;
	if (pitch > 89.0f) pitch = 89.0f;
	else if (pitch < -89.0f) pitch = -89.0f;
	update();
}

bool FreeCamera::LoadModel() {
	if(!scene.LoadModel("../data/player.obj")) {
		return false;
	}

	if(!scene.LoadTexture("../data/player.png")) {
		return false;
	}

	return true;
}
