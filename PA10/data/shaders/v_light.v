
#version 330

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_norm;
layout (location = 2) in vec2 v_texcood;

smooth out vec4 f_lighting;
smooth out vec2 f_texcoord;

#define MAX_LIGHTS 16
struct light {
	vec4 pos;
	vec3 diffuse_color, specular_color;
	float constant_attenuation, linear_attenuation, quadratic_attenuation;
	
	vec3 spotlight_direction;
	float spotlight_cutoff, spotlight_exponent;
};
uniform light lights[MAX_LIGHTS];
uniform int num_lights;

struct material {
	vec3 ambient, diffuse, specular;
	float shine;
};
uniform material object;

uniform vec3 ambient_color;
uniform sampler2D tex;
uniform mat4 model, view, proj;

void main() {

	vec4 pos = model * vec4(v_pos, 1.0);
	gl_Position = proj * view * pos;
	f_texcoord = v_texcood;

	vec3 norm_dir = normalize(inverse(transpose(mat3(model))) * v_norm);
	vec3 view_dir = normalize(vec3(inverse(view) * vec4(0,0,0,1) - pos)); // get direction to camera (0,0,0)
	vec3 light_dir;
	float attenuation;

	vec3 ambient = ambient_color * object.ambient;
	vec3 diffuse = vec3(0), specular = vec3(0);

	for(int i = 0; i < num_lights; i++) {

		if(lights[i].pos.w == 0) { 											// directional light
			
			attenuation = 1.0;
			light_dir = normalize(vec3(lights[i].pos));

		} else {															// point- or spot-light

			vec3 to_light = vec3(lights[i].pos - pos);
			float distance_to_light = length(to_light);
			light_dir = normalize(to_light);

			attenuation = 1.0 / (lights[i].constant_attenuation + lights[i].linear_attenuation * distance_to_light 
								 + lights[i].quadratic_attenuation * distance_to_light * distance_to_light);

			if(lights[i].spotlight_cutoff <= 90.0) {						// spotlight

				float clamp_cos = max(0.0, dot(-light_dir, lights[i].spotlight_direction));
				if(clamp_cos < cos(radians(lights[i].spotlight_cutoff))) {

					attenuation = 0.0;

				} else {

					attenuation = attenuation * pow(clamp_cos, lights[i].spotlight_exponent);
				}
			}
		}

		diffuse += attenuation * object.diffuse * lights[i].diffuse_color * max(0.0, dot(norm_dir, light_dir));
		if(dot(norm_dir, light_dir) >= 0.0) {								// is the face on the right side to face the light?

			float highlight = pow(max(0.0, dot(reflect(-light_dir, norm_dir), view_dir)), object.shine);
			specular += attenuation * lights[i].specular_color * object.specular * highlight;
		}
	}

	f_lighting = vec4(ambient + diffuse + specular, 1.0);
}