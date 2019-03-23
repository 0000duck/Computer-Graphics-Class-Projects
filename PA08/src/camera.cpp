
#include "camera.h"

OrbitCamera::OrbitCamera() {
	reset();
}

OrbitCamera::~OrbitCamera() {

}

void OrbitCamera::reset() {
	fov = 50.0f;
	yaw = 0.0f;
	pitch = 45.0f;
	radius = 20.0f;
	lookingAt = glm::vec3(0, 0, 0);
	update();
}

void OrbitCamera::update() {
	pos.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	pos.y = sin(glm::radians(pitch));
	pos.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	pos = radius * glm::normalize(pos) + lookingAt;
}

void OrbitCamera::move(int dx, int dy) {

	yaw += dx * sens;
	pitch -= dy * sens;
	if (yaw > 360.0f) yaw = 0.0f;
	else if (yaw < 0.0f) yaw = 360.0f;
	if (pitch > 89.0f) pitch = 89.0f;
	else if (pitch < -89.0f) pitch = -89.0f;
	update();
}

void OrbitCamera::setDistance(float d) {
	radius = d;
	update();
}

glm::mat4 OrbitCamera::GetProjection(float w, float h) {
  
	return glm::perspective(glm::radians(fov), w/h, 0.01f, 100.0f);
}

glm::mat4 OrbitCamera::GetViewWithoutTranslate() {

	return glm::lookAt(glm::vec3(0, 0, 0), pos, glm::vec3(0, 1, 0));
}

glm::mat4 OrbitCamera::GetView() {

	return glm::lookAt(pos, lookingAt, glm::vec3(0, 1, 0));
}

FreeCamera::FreeCamera() {
	reset();
}

FreeCamera::~FreeCamera() {

}

glm::mat4 FreeCamera::GetProjection(float w, float h) {
	return glm::perspective(glm::radians(fov), w/h, 0.01f, 100.0f);
}

glm::mat4 FreeCamera::GetView() {
	return glm::lookAt(pos, pos + front, up);
}

glm::mat4 FreeCamera::GetViewWithoutTranslate() {
	return glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f) + front, up);
}

void FreeCamera::reset() {
	fov = 60.0f;
	pitch = -45.0f;
	yaw = 225.0f;
	speed = 5.0f;
	pos = glm::vec3(5, 5, 5);
	globalUp = glm::vec3(0, 1, 0);
	update();
}

void FreeCamera::update() {
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);
	right = glm::normalize(glm::cross(front, globalUp));
	up = glm::normalize(glm::cross(right, front));
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
