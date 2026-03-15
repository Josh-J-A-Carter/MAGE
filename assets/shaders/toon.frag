#version 410 core

in vec3 f_col;
//in vec3 f_normal;
in float f_diffuse;

out vec4 FragColor;

void main() {
	float ambient = 0.1;

	const vec3 light_dir = vec3(0, 1, 0);
	float diffuse = f_diffuse;      //dot(light_dir, normalize(f_normal));
	
	float light = ambient + diffuse;
	
	const float band0 = 0.3;
	const float band1 = 0.6;
	const float band2 = 0.9;
	
	if (light > band1) light = band1;
	else if (light > band2) light = band2;
	else light = band0;

	FragColor = light * vec4(f_col.x, f_col.y, f_col.z, 1.0);
}