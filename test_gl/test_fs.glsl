#version 410

in vec3 position_eye, normal_eye;

uniform mat4 view_mat;
uniform vec3 light_position_world;

// fixed point light properties
// Ls, Ld, La are color. light_power determine how powerful the light is. 
vec3 Ls = vec3 (1.0, 1.0, 1.0); //
vec3 Ld = vec3 (1.0, 1.0, 1.0); //
vec3 La = vec3 (1.0, 1.0, 1.0); // 
float light_power = 10.0f; // just one augmentation.
  
// surface reflectance
vec3 Ks = vec3 (0.2, 0.2, 0.2); // 
vec3 Kd = vec3 (0.9, 0.8, 0.2); // 
vec3 Kd_inside = vec3(0.75, 0.75, 0.5);
vec3 Ka = vec3(0.3, 0.3, 0.3) * Kd;
vec3 Ka_inside = vec3(0.3, 0.3, 0.3) * Kd_inside; 
float specular_exponent = 5.0; // specular 'power'

out vec4 fragment_colour; // final colour of surface

void main () {
	vec3 surface_to_viewer_eye = normalize (-position_eye);
	bool is_inside = (dot(surface_to_viewer_eye, normal_eye)) < 0.0f ? true : false;
	

	// ambient intensity
	vec3 Ia;
	if (is_inside) {
		Ia = La * Ka_inside;
	} else {
		Ia = La * Ka;
	}

	// diffuse intensity
	// raise light position to eye space
	vec3 light_position_eye = vec3 (view_mat * vec4 (light_position_world, 1.0));
	vec3 distance_to_light_eye = light_position_eye - position_eye;
	float distance = length(distance_to_light_eye);
	vec3 direction_to_light_eye = normalize (distance_to_light_eye);
	float dot_prod = dot (direction_to_light_eye, normal_eye);
	// dot_prod = max (dot_prod, 0.0);
	dot_prod = abs(dot_prod); // To make the color of both side the same. 
	vec3 Id;
	if (is_inside) {
		Id = Ld * Kd_inside * dot_prod * light_power / (distance * distance); // final diffuse intensity	
	} else {
		Id = Ld * Kd * dot_prod * light_power / (distance * distance); // final diffuse intensity
	}
	

	// specular intensity
	// origin Phong Model
	//vec3 reflection_eye = reflect (-direction_to_light_eye, normal_eye);
	//float dot_prod_specular = dot (reflection_eye, surface_to_viewer_eye);
	//dot_prod_specular = max (dot_prod_specular, 0.0);
	//float specular_factor = pow (dot_prod_specular, specular_exponent);
	
	// blinn
	vec3 half_way_eye = normalize (surface_to_viewer_eye + direction_to_light_eye);
	float dot_prod_specular = dot (half_way_eye, normal_eye);
	// dot_prod_specular = max(dot_prod_specular, 0.0);
	dot_prod_specular = abs(dot_prod_specular); // We want to make both face reflect light. 
	float specular_factor = pow (dot_prod_specular, specular_exponent);
	
	vec3 Is = Ls * Ks * specular_factor * light_power / (distance * distance); // final specular intensity
	

	// final colour
	fragment_colour = vec4 (Is + Id + Ia, 1.0); // TODO: I have a question. What if it is larger than 1.0 after adding. 
	// fragment_colour = vec4(dot_prod, 0.0, 0.0, 1.0);
}
