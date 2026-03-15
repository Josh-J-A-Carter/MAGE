#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 v_tex;
layout(location = 2) in vec3 v_col;

uniform mat4 u_proj;
uniform mat4 u_rot;
uniform vec3 u_color;

//out vec3 f_normal;
out vec3 f_col;
out float f_diffuse;

void main() {
	gl_Position = u_proj * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	vec3 light_dir = vec3(0.0, 1.0, 0.0);
    
	vec3 worldspace_normal = (u_rot * vec4(v_col.xyz, 1.0)).xyz;
	f_col = u_color;
	f_diffuse = dot(light_dir, worldspace_normal);

	// f_normal = (u_rot * vec4(v_col.xyz, 1.0)).xyz;
	// f_col = u_color;
}