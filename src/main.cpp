#include "ecs.h"

#include "gmath.h"

#include "glad/glad.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include <iostream>

constexpr std::size_t MAX_ENTITIES { 1000 };
constexpr std::size_t MAX_COMPONENTS { 32 };

using State = ECS<MAX_ENTITIES, MAX_COMPONENTS>;

struct Transform {};
struct Name { std::string name; };
struct Rating { int coolness; };

void my_system(State ecs) {
	auto [ transforms, names ] = ecs.components<Transform, Name>();
	const auto& entities = ecs.entities({ my_system });

	std::cout << "my system :)" << std::endl;
	
	for (auto& entity : entities) {
		std::cout << "Name: " << names[entity.index].name << std::endl;
		names[entity.index].name += ".";
	}

	std::cout << std::endl;
}



int main(int argc, char** argv) {

	//std::cout << res.x << ", " << res.y << ", " << res.z << ", " << res.w << std::endl;

	//std::cout << test[0] << ", " << test[4] << ", " << test[8] << ", " << test[12] << std::endl;
	//std::cout << test[1] << ", " << test[5] << ", " << test[9] << ", " << test[13] << std::endl;
	//std::cout << test[2] << ", " << test[6] << ", " << test[10] << ", " << test[14] << std::endl;
	//std::cout << test[3] << ", " << test[7] << ", " << test[11] << ", " << test[15] << std::endl;

	//State ecs {};

	//ecs.register_component<Transform>();
	//ecs.register_component<Name>();

	//Entity e1 { ecs.create_entity() };
	//ecs.add_component<Name>(e1, { "Cool Name" } );
	//ecs.add_component<Transform>(e1, {});

	//Entity e2 { ecs.create_entity() };
	//ecs.add_component<Name>(e2, { "Really Cool Name" });

	//ecs.register_system<Name, Transform>({ my_system });

	//my_system(ecs);

	//ecs.add_component<Transform>(e2, {});

	//my_system(ecs);
	//my_system(ecs);
	//my_system(ecs);
	//my_system(ecs);

	//ecs.remove_component<Name>(e1);

	//my_system(ecs);

	//ecs.remove_component<Transform>(e2);

	//my_system(ecs);

	//Entity e3 { ecs.create_entity() };
	//ecs.add_component<Name>(e3, { "Mega Cool Name" });
	//Entity e4 { ecs.create_entity() };
	//Entity e5 { ecs.create_entity() };
	//ecs.add_component<Transform>(e5, {});
	//
	//my_system(ecs);

	//return 0;

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) return EXIT_FAILURE;

	constexpr int WIDTH { 640 };
	constexpr int HEIGHT { 480 };

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	constexpr SDL_WindowFlags FLAGS { SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_OPENGL };

	SDL_Window* window = SDL_CreateWindow("SDL3 window", WIDTH, HEIGHT, FLAGS);

	SDL_GLContext openGL_context = SDL_GL_CreateContext(window);
	if (!openGL_context) {
		std::cout << "Oops." << std::endl;
		return EXIT_FAILURE;
	}

	GLADloadproc loadproc = static_cast<GLADloadproc>(static_cast<void*>(SDL_GL_GetProcAddress));
	if (!gladLoadGLLoader(loadproc)) {
		std::cout << "GLAD could not be initialised :( SDL_Error: " << SDL_GetError() << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Successfully created OpenGL Context" << std::endl;


	float vertices[] {
		// Front vertices
		-1, -1, -1,
		1, -1, -1,
		1, 1, -1,
		-1, 1, -1,

		// Back vertices
		-1, -1, -3,
		1, -1, -3,
		1, 1, -3,
		-1, 1, -3,
	};

	int indices[] {
		// Front
		0, 1, 2,
		2, 3, 0,

		//// Top
		3, 2, 6,
		6, 7, 3
	};

	constexpr int NUM_INDICES { sizeof(indices) / sizeof(indices[0]) };

	GLuint ebo {};
	glGenBuffers(1, &ebo);

	GLuint vbo {};
	glGenBuffers(1, &vbo);

	GLuint vao {};
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vertices[0]), (void*) 0);
	glEnableVertexAttribArray(0);

	GLuint program { glCreateProgram() };

	const char* vertex_shader {
		"#version 410 core												\n\
		layout(location = 0) in vec3 aPos;								\n\
		uniform mat4 u_proj;											\n\
																		\n\
		void main() {													\n\
			gl_Position = u_proj * vec4(aPos.x, aPos.y, aPos.z, 1.0);	\n\
		}"
	};

	const char* fragment_shader {
		"#version 410 core												\n\
		out vec4 FragColor;												\n\
																		\n\
		void main() {													\n\
			FragColor = vec4(1.0, 1.0, 1.0, 1.0);						\n\
		}"
	};

	int  success;
	char infoLog[512];

	GLuint vs { glCreateShader(GL_VERTEX_SHADER) };
	glShaderSource(vs, 1, &vertex_shader, nullptr);
	glCompileShader(vs);

	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	
	if(!success) {
		glGetShaderInfoLog(vs, 512, NULL, infoLog);
		std::cout << "ERROR: Vertex shader compilation failed\n" << infoLog << std::endl;
	}

	GLuint fs { glCreateShader(GL_FRAGMENT_SHADER) };
	glShaderSource(fs, 1, &fragment_shader, nullptr);
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(vs, 512, NULL, infoLog);
		std::cout << "ERROR: Fragment shader compilation failed\n" << infoLog << std::endl;
	}

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	glUseProgram(program);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	Mat4x4 cam { view({0, 0, -0.5}, {0, -1, -10}, {0, 1, 0}) };
	Mat4x4 proj { perspective(1.78f, 160.0f, 0.1f, 100.0f) * cam };

	GLint u_proj_loc { glGetUniformLocation(program, "u_proj") };
	glUniformMatrix4fv(u_proj_loc, 1, GL_FALSE, &proj);


	while (true) {
		int width;
		int height;

		SDL_GetWindowSize(window, &width, &height);

		glViewport(0, 0, width, height);
		glClearColor(0.7f, 0.7f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glDrawElements(GL_TRIANGLES, NUM_INDICES, GL_UNSIGNED_INT, 0);

		SDL_GL_SwapWindow(window);
	}

	SDL_Quit();

	return EXIT_SUCCESS;

}