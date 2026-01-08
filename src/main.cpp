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


Vec3 looking_at(float theta, float phi) {
	return { cos(theta) * sin(phi), -sin(theta), -cos(theta) * cos(phi) };
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
		-1, -1, -1,    1.0f, 1.0f, 1.0f,
		1, -1, -1,		0.6f, 1.0f, 1.0f,
		1, 1, -1,		1.0f, 0.6f, 1.0f,
		-1, 1, -1,		1.0f, 1.0f, 0.6f,

		// Back vertices
		-1, -1, -3,		1.0f, 0.6f, 0.6f,
		1, -1, -3,		0.6f, 1.0f, 0.6f,
		1, 1, -3,		0.6f, 0.6f, 1.0f,
		-1, 1, -3,		0.3f, 0.3f, 0.3f
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
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(vertices[0]), (void*) (0 * sizeof(vertices[0])));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(vertices[0]), (void*) (3 * sizeof(vertices[0])));
	glEnableVertexAttribArray(1);


	GLuint program { glCreateProgram() };

	const char* vertex_shader {
		"#version 410 core												\n\
		layout(location = 0) in vec3 aPos;								\n\
		layout(location = 1) in vec3 v_col;								\n\
		uniform mat4 u_proj;											\n\
		out vec3 f_col;													\n\
																		\n\
		void main() {													\n\
			gl_Position = u_proj * vec4(aPos.x, aPos.y, aPos.z, 1.0);	\n\
			f_col = v_col;												\n\
		}"
	};

	const char* fragment_shader {
		"#version 410 core												\n\
		in vec3 f_col;													\n\
		out vec4 FragColor;												\n\
																		\n\
		void main() {													\n\
			FragColor = vec4(f_col.x, f_col.y, f_col.z, 1.0);			\n\
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

	SDL_SetWindowRelativeMouseMode(window, true);

	float theta { 0 };
	float phi { 0 };
	Vec3 pos {};

	constexpr float PI { 3.14159f };
	constexpr float MAX_THETA { 0.8f * PI / 2 };

	const bool* keyboard { SDL_GetKeyboardState(NULL) };
	
	while (true) {
		// Window updates
		int width;
		int height;

		SDL_GetWindowSize(window, &width, &height);

		// Input
		SDL_Event evt {};
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_EVENT_MOUSE_MOTION) {
				theta += evt.motion.yrel / 500;
				phi += evt.motion.xrel / 500;

				if (theta > MAX_THETA) theta = MAX_THETA;
				if (theta < -MAX_THETA) theta = -MAX_THETA;
			}

			if (evt.type == SDL_EVENT_QUIT) {
				return EXIT_SUCCESS;
			}
		}

		if (SDL_GetMouseFocus() == window) {
			SDL_WarpMouseInWindow(window, width / 2.0f, height / 2.0f);
			SDL_HideCursor();
		}

		constexpr Vec3 up { 0, 1, 0 };
		Vec3 facing { looking_at(theta, phi) };
		Vec3 right { up.cross(-facing).normalised() };

		pos += Vec3{ 0, keyboard[SDL_SCANCODE_SPACE] ? 1.0f / 500 : 0, 0 };
		pos += Vec3{ 0, keyboard[SDL_SCANCODE_LSHIFT] ? -1.0f / 500 : 0, 0 };

		pos += keyboard[SDL_SCANCODE_W] ? facing * (1.0f / 500) : Vec3{};
		pos += keyboard[SDL_SCANCODE_S] ? facing * (-1.0f / 500) : Vec3{};

		pos += keyboard[SDL_SCANCODE_D] ? right * (1.0f / 500) : Vec3{};
		pos += keyboard[SDL_SCANCODE_A] ? right * (-1.0f / 500) : Vec3{};

		// Camera / Uniforms
		Mat4x4 cam { view(pos, facing, up)};
		Mat4x4 proj { perspective(static_cast<float>(width) / height, 120.0f, 0.1f, 100.0f) * cam };

		GLint u_proj_loc{ glGetUniformLocation(program, "u_proj") };
		glUniformMatrix4fv(u_proj_loc, 1, GL_FALSE, &proj);

		// Render
		glViewport(0, 0, width, height);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glDrawElements(GL_TRIANGLES, NUM_INDICES, GL_UNSIGNED_INT, 0);

		SDL_GL_SwapWindow(window);
	}

	SDL_Quit();

	return EXIT_SUCCESS;

}