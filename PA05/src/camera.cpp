#include "camera.h"

Camera::Camera() {
	reset();
}

Camera::~Camera() {

}

void Camera::reset() {
	yaw = 0.0f;
	pitch = 45.0f;
	radius = 10.0f;
	lookingAt = glm::vec3(0, 0, 0);
	update();
}

void Camera::update() {
	pos.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	pos.y = sin(glm::radians(pitch));
	pos.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	pos = radius * glm::normalize(pos) + lookingAt;
}

void Camera::move(int dx, int dy) {

	yaw += dx * sens;
	pitch -= dy * sens;
	if (yaw > 360.0f) yaw = 0.0f;
	else if (yaw < 0.0f) yaw = 360.0f;
	if (pitch > 89.0f) pitch = 89.0f;
	else if (pitch < -89.0f) pitch = -89.0f;
	update();
}

glm::mat4 Camera::GetProjection(float w, float h) {
  
	return glm::perspective(45.0f, w/h, 0.01f, 100.0f);
}

glm::mat4 Camera::GetView() {

	return glm::lookAt(pos, lookingAt, glm::vec3(0, 1, 0));
}
