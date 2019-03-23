#ifndef CAMERA_H
#define CAMERA_H

#include "graphics_headers.h"

// enumeration of camera types
enum class CameraType : int {
	orbit = 0,
	free,
};

// Interface for types of cameras
class Camera {
protected:
	virtual glm::mat4 GetProjection(float w, float h) = 0;
	virtual glm::mat4 GetView() = 0;
	// for the skybox transform
	virtual glm::mat4 GetViewWithoutTranslate() = 0;

	// reset to default values
	virtual void reset() = 0;
	// update vectors based on move-able values
	virtual void update() = 0;
	// move direction based on mouse x & y deltas
	virtual void move(int dx, int dy) = 0;
	// for orbit - set distance away from center
	virtual void setDistance(float d) = 0;

	CameraType type;
	const float sens = 0.5f;
	float fov;
	glm::vec3 pos;
	bool lock = false;

	friend class Graphics;
};

// Camera that orbits on a sphere around a point
class OrbitCamera : public Camera {

public:
	OrbitCamera();
	~OrbitCamera();

	glm::mat4 GetProjection(float w, float h);
	glm::mat4 GetView();
	glm::mat4 GetViewWithoutTranslate();
	void reset();
	void update();
	void move(int dx, int dy);
	void setDistance(float d);

private:
	glm::vec3 lookingAt;
	float pitch, yaw, radius;

	friend class Graphics;
};

// Free-moving first-person camera
class FreeCamera : public Camera {

public:
	FreeCamera();
	~FreeCamera();

	glm::mat4 GetProjection(float w, float h);
	glm::mat4 GetView();
	glm::mat4 GetViewWithoutTranslate();
	void reset();
	void update();
	void move(int dx, int dy);
	void setDistance(float d) {}

private:
	unsigned int last_update;
	glm::vec3 front, up, right, globalUp;
	float pitch, yaw, speed;

	friend class Graphics;
};

#endif /* CAMERA_H */
