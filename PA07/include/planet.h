
#ifndef PLANET_H
#define PLANET_H

#include <vector>
#include <string>
#include <picojson.h>
#include "scene.h"

struct ss_settings;
struct MatLocs;

class Planet {

public:
	Planet();
	Planet(std::string json);
	~Planet();

	// get model matrix based on current attributes
	glm::mat4 getModel();
	// get position based on current attributes
	glm::vec3 getPos();

private:
	friend class SolarSystem;

	// render planet, moons, rings based on settings
	void Render(const MatLocs& matlocs, const ss_settings& settings);
	// render orbit path
	void RenderPath(GLint shaderMdlmx);
	// simulate planet movement over dT based on settings
	void Update(unsigned int dT, const ss_settings& settings, glm::mat4 translation = glm::mat4(1.0f));
	// load model/textures into planet scene
	bool LoadScene();
	// free planet scene assets
	void DeleteScene();

	// load attributes from JSON string
	bool LoadJSON(std::string json);
	// load attributes from JSON object
	void LoadJSONObj(const picojson::object& obj);
	// update current planet movement angles, called from Update
	void UpdateAngles(unsigned int dT, const ss_settings& settings);

	struct ScaleResult {
		float orbit_radius, scaled_diameter, scaled_inclination_orbit;
	};
	// get neccesary values for modifying simulation settings
	ScaleResult ScaleForSettings(const ss_settings& settings);

	// generate orbit trace to highlight orbit path
	void GenerateOrbitTrace(const ss_settings& settings); 

	// ring structure
	struct Ring {
		std::string model, texture;
		float tilt;
		Scene scene;
	};

	// attributes
	std::string name;
	std::string model, texture;

		// relative to earth
	float diameter, distance, orbital_period, rotation_period;

		// absolute
	float rank, inclination_orbit, inclination_equator, orbital_eccentricity;

	// transformations
	glm::mat4 parentmx, scalemx, rotatemx, tiltmx, orbittiltmx, translatemx;
	glm::vec3 pos_for_tracking;

	// children
	std::vector<Planet> moons;
	std::vector<Ring> rings;

	// for tracing orbit & movement paths
	std::vector<glm::vec3> orbit_trace;
	GLuint orbit_trace_vao, orbit_trace_vbo;

	std::vector<glm::vec3> movement_trace;

	// scene
	Scene scene;

	// current position
	float current_orbit_angle	= 0.0f;
	float current_rotate_angle 	= 0.0f;
};

#endif // PLANET_H
