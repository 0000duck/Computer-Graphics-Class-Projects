
#include "solarsystem.h"
#include "planet.h"
#include "graphics.h"
#include <cmath>

Planet::Planet() {
	orbit_trace_vbo = 0;
	diameter = distance = orbital_period = rotation_period = 0.0f;
	inclination_orbit = inclination_equator = orbital_eccentricity = 0.0f;
}

Planet::Planet(std::string json) {
	orbit_trace_vbo = 0;
	diameter = distance = orbital_period = rotation_period = 0.0f;
	inclination_orbit = inclination_equator = orbital_eccentricity = 0.0f;

	LoadJSON(json);
}

Planet::~Planet() {

	moons.clear();
	rings.clear();
}

void Planet::DeleteScene() {
	scene.DeleteMesh();
	scene.DeleteTexture();
	for(auto& r : rings) {
		r.scene.DeleteMesh();
		r.scene.DeleteTexture();
	}
	for(auto& m : moons) {
		m.DeleteScene();
	}
	if(orbit_trace_vbo) {
		glDeleteBuffers(1, &orbit_trace_vbo);
	}
}

bool Planet::LoadScene() {

	if(!model.size() || !texture.size()) {
		std::cerr << "Planet model/texture not initialized!" << std::endl;
		return false;
	}

	if(!scene.LoadModel(model)) {
		return false;
	}

	if(!scene.LoadTexture(texture)) {
		return false;
	}

	for(auto& i : rings) {
		if(!i.scene.LoadModel(i.model)) {
			return false;
		}

		if(!i.scene.LoadTexture(i.texture)) {
			return false;
		}
	}

	for(auto& m : moons) {
		m.LoadScene();
	}

	return true;
}

bool Planet::LoadJSON(std::string json) {

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
	LoadJSONObj(obj);

	return true;
}

#define stringfield(name) if(i.first == #name) name = i.second.to_str();
#define floatfield(name) if(i.first == #name && i.second.is<double>()) name = i.second.get<double>();

void Planet::LoadJSONObj(const picojson::object& obj) {

	for (auto& i : obj) {

		stringfield(name);
		stringfield(model);
		stringfield(texture);

		floatfield(rank);
		floatfield(diameter);
		floatfield(distance);
		floatfield(orbital_period);
		floatfield(rotation_period);
		floatfield(inclination_orbit);
		floatfield(inclination_equator);
		floatfield(orbital_eccentricity);
		floatfield(current_orbit_angle);

		if(i.first == "moons" && i.second.is<picojson::array>()) {

			const auto& moon_list = i.second.get<picojson::array>();
			for(auto& j : moon_list) {

				if(j.is<picojson::object>()) {

					Planet m;
					m.rank = 3;
					m.LoadJSONObj(j.get<picojson::object>());
					m.LoadScene();
					moons.push_back(m);
				}
			}
		}

		if(i.first == "rings" && i.second.is<picojson::array>()) {

			const auto& ring_list = i.second.get<picojson::array>();
			for(auto& j : ring_list) {

				if(j.is<picojson::object>()) {
					Ring r;

					const auto& ring = j.get<picojson::object>();
					for(auto& k : ring) {
						if(k.first == "model") {
							r.model = k.second.to_str();
						} else if(k.first == "texture") {
							r.texture = k.second.to_str();
						} else if(k.first == "tilt" && k.second.is<double>()) {
							r.tilt = k.second.get<double>();
						}
					}
					rings.push_back(r);
				}
			}
		}
	}
}

glm::mat4 Planet::getModel() {
	return parentmx * orbittiltmx * translatemx * rotatemx * tiltmx * scalemx;
}

glm::vec3 Planet::getPos() {

	if(name == "Sun") return glm::vec3(1.0f);
	return pos_for_tracking;
}

void Planet::GenerateOrbitTrace(const ss_settings& settings) {

	orbit_trace.clear();
	ScaleResult s = ScaleForSettings(settings);

	for(float segment = 0; segment < 360; segment += 2) {

		glm::vec3 begin(s.orbit_radius*cos(glm::radians(segment)), 0.0 , s.orbit_radius*sin(glm::radians(segment)));
		glm::vec3 end(s.orbit_radius*cos(glm::radians(segment + 1.0f)), 0.0 , s.orbit_radius*sin(glm::radians(segment + 1.0f)));
		orbit_trace.push_back(begin);
		orbit_trace.push_back(end);
	}

	if(!orbit_trace_vbo) {
		glGenBuffers(1, &orbit_trace_vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, orbit_trace_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * orbit_trace.size(), &orbit_trace[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	for(auto& m : moons) {
		m.GenerateOrbitTrace(settings);
	}
}

void Planet::RenderPath(GLint shaderMdlmx) {

	glm::mat4 trans = parentmx * orbittiltmx;

	glBindBuffer(GL_ARRAY_BUFFER, orbit_trace_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glUniformMatrix4fv(shaderMdlmx, 1, GL_FALSE, glm::value_ptr(trans));
	glDrawArrays(GL_LINES, 0, 360);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);

	for(auto& m : moons) {
		m.RenderPath(shaderMdlmx);
	}
}

void Planet::Render(const MatLocs& matlocs, const ss_settings& settings) {

	for(auto& r : rings) {

		glm::mat4 tilt = glm::rotate(glm::mat4(1.0f), glm::radians(r.tilt), glm::vec3(1.0f, 0.0f, 1.0f));
		glm::mat4 modelmx = parentmx * orbittiltmx * translatemx * tilt * scalemx;

		glUniform1f(matlocs.ambient, 0.2f);
		glUniformMatrix4fv(matlocs.model, 1, GL_FALSE, glm::value_ptr(modelmx));
		glUniformMatrix4fv(matlocs.rotate, 1, GL_FALSE, glm::value_ptr(orbittiltmx * tilt));

		r.scene.Render();
	}

	for(auto& m : moons) {
		m.Render(matlocs, settings);
	}

	if(name == "Sun")
		glUniform1f(matlocs.ambient, 1.0f);
	else
		glUniform1f(matlocs.ambient, 0.2f);
	glUniformMatrix4fv(matlocs.rotate, 1, GL_FALSE, glm::value_ptr(orbittiltmx * rotatemx * tiltmx));
	glUniformMatrix4fv(matlocs.model, 1, GL_FALSE, glm::value_ptr(getModel()));

	scene.Render();
}

Planet::ScaleResult Planet::ScaleForSettings(const ss_settings& settings) {

	ScaleResult s;
	if(settings.do_distance_scale) {
		s.scaled_diameter = diameter * settings.distance_scale * settings.diameter_scale;
		s.orbit_radius = distance * settings.distance_scale / 2.0f;
		s.scaled_inclination_orbit = inclination_orbit;
	} else {
		s.orbit_radius = 2.5f * rank;
		s.scaled_diameter = 4.0f * (0.05f + diameter / 20.0f);
		if(name == "Sun") {
			s.scaled_diameter = 1.8f;
			s.orbit_radius = 0.0f;
		}
		s.scaled_inclination_orbit = inclination_orbit / 2.0f;
	}
	return s;
}

void Planet::Update(unsigned int dT, const ss_settings& settings, glm::mat4 translation) {

	UpdateAngles(dT, settings);

	ScaleResult s = ScaleForSettings(settings);

	parentmx = translation;
  	scalemx = glm::scale(glm::mat4(1.0f), glm::vec3(s.scaled_diameter));
  	rotatemx = glm::rotate(glm::mat4(1.0f), current_rotate_angle, glm::vec3(0.0, 1.0, 0.0));
  	tiltmx = glm::rotate(glm::mat4(1.0f), glm::radians(inclination_equator), glm::vec3(1.0, 0.0, 1.0));

  	glm::vec3 pos = glm::vec3(s.orbit_radius*cos(current_orbit_angle), 0.0 , s.orbit_radius*sin(current_orbit_angle));
	translatemx = glm::translate(glm::mat4(1.0f), pos);

  	orbittiltmx = glm::rotate(glm::mat4(1.0f), glm::radians(s.scaled_inclination_orbit), glm::vec3(1.0, 0.0, 1.0));

	pos_for_tracking = pos + (3.0f * s.scaled_diameter * glm::normalize(pos)) + glm::vec3(0, s.scaled_diameter, 0);
	pos_for_tracking = glm::vec3(orbittiltmx * glm::vec4(pos_for_tracking, 1.0f));

	for(auto& m : moons) {
		m.Update(dT, settings, orbittiltmx * translatemx);
	}
}

void Planet::UpdateAngles(unsigned int dT, const ss_settings& settings) {

	float multiplier      = (settings.do_time_scale ? settings.time_scale : SEC_TO_EARTH_DAYS(1.0f));
	current_orbit_angle  += (float)dT / 1000.0f * multiplier * (orbital_period ? 1.0f / orbital_period : 0);
	current_rotate_angle += (float)dT / 1000.0f * multiplier * (rotation_period ? 1.0f / rotation_period : 0);
}
