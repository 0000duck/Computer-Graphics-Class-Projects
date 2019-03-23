
#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include "planet.h"
#include <vector>
#include <string>

// unit conversions
#define EARTH_DIAMETERS_TO_KM(d) (d * 12756.0f) 
#define KM_TO_EARTH_DIAMETERS(k) (k / 12756.0f) 

#define EARTH_DAYS_TO_SEC(d) (d * 24.0f * 3600.0f)
#define SEC_TO_EARTH_DAYS(s) (s / 24.0f / 3600.0f)

// solar system simulation/rendering settings
struct ss_settings {
	float time_scale 		= 100.0f;	// factor to scale simulation dT
	float distance_scale 	= 0.05f;	// factor to scale all distances
	float diameter_scale	= 25.0f;	// factor to scale planet diameters - also takes into account distance_scale
	bool do_time_scale		= true;		// use time_scale or use real-time (incredibly slow)
	bool do_distance_scale	= false;	// use distance_scale or use accurate relative distances (incredibly huge) - independent of diameter_scale
	bool do_orbit_path		= true;		// render orbit paths
	int track_planet_idx 	= 0;		// planet currently being tracked by tracking camera
};

struct MatLocs;

class SolarSystem {

public:
	SolarSystem();
	~SolarSystem();

	// load planet JSON definitions from directory
	bool LoadPlanets(std::string dir);
	// update system simulation by dT
	void Update(unsigned int dT);	
	// generate planet paths
	void GenPaths();
	// render planets
	void Render(const MatLocs& matlocs);
	// render planet paths
	void RenderPaths(GLint shaderMdlmx);
	// render UI
	void UI(bool track);
	// get relative scale to set how far away the orbiting camera orbits
	float GetCameraScale();
	// get the currently tracked planet based on track_planet_idx
	Planet* GetTrackedPlanet();

private:

	ss_settings settings;
	std::vector<Planet> planets;
};

#endif // SOLARSYSTEM_H
