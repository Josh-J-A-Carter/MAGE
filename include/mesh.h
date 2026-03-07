#ifndef MESH_H
#define MESH_H

#include "glad/glad.h"

#include <vector>

struct Mesh {
	GLuint ebo {};
	int num_indices {};

	Mesh(std::vector<float>& vertices, std::vector<int>& indices);
};

#endif