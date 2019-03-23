#ifndef CAMERA_H
#define CAMERA_H

#include "graphics_headers.h"

class Camera {

public:
	Camera();
	~Camera();
	glm::mat4 GetProjection(float w, float h);
	glm::mat4 GetView();
 	
	void reset();
	void update();
	void move(int dx, int dy);

	const float sens = 0.5f;

private:
	glm::vec3 pos, lookingAt;
	float pitch, yaw, radius;
};

#endif /* CAMERA_H */
