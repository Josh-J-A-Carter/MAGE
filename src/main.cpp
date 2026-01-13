#include "ecs.h"

#include "gmath.h"

#include "glad/glad.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include <iostream>
#include <random>

constexpr std::size_t MAX_ENTITIES { 1000 };
constexpr std::size_t MAX_COMPONENTS { 32 };

using State = ECS<MAX_ENTITIES, MAX_COMPONENTS>;

struct Transform {
	Vec3 position;
	Quat orientation;

	Mat4x4 model() {
		Mat4x4 translate {};

		translate[0] = 1;
		translate[5] = 1;
		translate[10] = 1;
		translate[15] = 1;

		translate[12] = position.x;
		translate[13] = position.y;
		translate[14] = position.z;

		// Rotate in local space, and *then* translate
		Mat4x4 rot { orientation.matrix() };

		return translate * rot;
	}
};

struct Application {
	SDL_Window* window;
	GLuint shader;
	int width;
	int height;
	Entity player;
	std::mt19937 rand;
};

struct Mesh {
	GLuint ebo {};
	int num_indices {};
};

struct MeshRenderer {
	std::shared_ptr<Mesh> mesh {};
	Vec3 color {};
};

struct PhysicsBody {
	float speed {};
};

struct Camera {
	float fov;
	float near;
	float far;

	float theta;
	float phi;

	Vec3 up;
	Vec3 facing;
	Vec3 right;
};


bool input_system(State& ecs, Application& app);
void physics_system(State& ecs, Application& app);
void render_system(State& ecs, Application& app);


int main(int argc, char** argv) {

	//std::cout << res.x << ", " << res.y << ", " << res.z << ", " << res.w << std::endl;

	//std::cout << test[0] << ", " << test[4] << ", " << test[8] << ", " << test[12] << std::endl;
	//std::cout << test[1] << ", " << test[5] << ", " << test[9] << ", " << test[13] << std::endl;
	//std::cout << test[2] << ", " << test[6] << ", " << test[10] << ", " << test[14] << std::endl;
	//std::cout << test[3] << ", " << test[7] << ", " << test[11] << ", " << test[15] << std::endl;


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
		std::cout << "Failed to create OpenGL context" << std::endl;
		return EXIT_FAILURE;
	}

	GLADloadproc loadproc = static_cast<GLADloadproc>(static_cast<void*>(SDL_GL_GetProcAddress));
	if (!gladLoadGLLoader(loadproc)) {
		std::cout << "GLAD could not be initialised. SDL_Error: " << SDL_GetError() << std::endl;
		return EXIT_FAILURE;
	}


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

		// Back
		5, 4, 7,
		5, 7, 6,

		// Top
		3, 2, 6,
		6, 7, 3,

		// Bottom
		1, 0, 4,
		4, 5, 1,

		// Left
		0, 3, 7,
		0, 7, 4,

		// Right
		1, 6, 2,
		1, 5, 6,
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

	std::shared_ptr<Mesh> cube_mesh { std::make_shared<Mesh>(ebo, NUM_INDICES) };

	GLuint program { glCreateProgram() };

	const char* vertex_shader {
		"#version 410 core												\n\
		layout(location = 0) in vec3 aPos;								\n\
		layout(location = 1) in vec3 v_col;								\n\
		uniform mat4 u_proj;											\n\
		uniform vec3 u_color;											\n\
		out vec3 f_col;													\n\
																		\n\
		void main() {													\n\
			gl_Position = u_proj * vec4(aPos.x, aPos.y, aPos.z, 1.0);	\n\
			f_col = v_col * u_color;									\n\
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

	Application app { .window = window, .shader = program, .rand = std::mt19937 { std::random_device{}() } };


	State ecs {};

	ecs.register_component<Transform>();
	ecs.register_component<Camera>();
	ecs.register_component<PhysicsBody>();
	ecs.register_component<MeshRenderer>();

	ecs.register_system<Transform, MeshRenderer>(render_system);
	ecs.register_system<Transform, PhysicsBody>(physics_system);

	Entity player { ecs.create_entity() };
	ecs.add_component<Transform>(player, { 0, 0, 0 });
	ecs.add_component<Camera>(player, { .fov = 90, .near = 0.1f, .far = 100.0f, .theta = 0, .phi = 0 });

	app.player = player;


	for (int i { 0 } ; i < 1 ; i += 1) {
		Entity cube { ecs.create_entity() };
		ecs.add_component<MeshRenderer>(cube, { cube_mesh, Vec3::rand(app.rand, 0.1f, 1.0f) });
		ecs.add_component<PhysicsBody>(cube, { 0 });

		constexpr float PI { 3.14159f };
		float theta = std::uniform_real_distribution{ -PI/2, PI/2 }(app.rand);
		float phi = std::uniform_real_distribution{ 0.0f, 2 * PI }(app.rand);
		Vec3 rot_axis = { cos(theta) * cos(phi), cos(theta) * sin(phi), sin(theta) };

		float rot_amount = std::uniform_real_distribution{ 0.0f, 2 * PI }(app.rand);

		Quat orientation { Quat::rotation(rot_axis, rot_amount) };
		ecs.add_component<Transform>(cube, { Vec3::rand(app.rand, -20, 20) + Vec3{ 0, 0, -30 }, orientation });
	}
	
	bool quit = false;
	while (!quit) {
		quit = input_system(ecs, app);
		physics_system(ecs, app);
		render_system(ecs, app);
	}

	SDL_Quit();

	return EXIT_SUCCESS;

}

Vec3 looking_at(float theta, float phi) {
	return { cos(theta) * sin(phi), -sin(theta), -cos(theta) * cos(phi) };
}

bool input_system(State& ecs, Application& app) {
	constexpr float PI { 3.14159f };
	constexpr float MAX_THETA { 0.8f * PI / 2 };
	constexpr float SPEED { 5.0f };

	auto [ transforms, cameras ] = ecs.components<Transform, Camera>();
	Transform& player_transform { transforms[app.player.index] };
	Camera& player_camera { cameras[app.player.index] };

	SDL_GetWindowSize(app.window, &app.width, &app.height);
	
	const bool* keyboard { SDL_GetKeyboardState(NULL) };

	SDL_Event evt {};
	while (SDL_PollEvent(&evt)) {
		if (evt.type == SDL_EVENT_MOUSE_MOTION) {
			player_camera.theta += evt.motion.yrel / 500;
			player_camera.phi += evt.motion.xrel / 500;

			if (player_camera.theta > MAX_THETA) player_camera.theta = MAX_THETA;
			if (player_camera.theta < -MAX_THETA) player_camera.theta = -MAX_THETA;
		}

		if (evt.type == SDL_EVENT_QUIT) {
			return true;
		}
	}

	if (SDL_GetMouseFocus() == app.window) {
		SDL_WarpMouseInWindow(app.window, app.width / 2.0f, app.height / 2.0f);
		SDL_HideCursor();
	}

	Vec3 up = { 0, 1, 0 };
	Vec3 facing = looking_at(player_camera.theta, player_camera.phi);
	Vec3 right = up.cross(-facing).normalised();

	player_camera.up = up;
	player_camera.facing = facing;
	player_camera.right = right;

	player_transform.position += Vec3 { 0, keyboard[SDL_SCANCODE_SPACE] ? 1.0f / 500 : 0, 0 } * SPEED;
	player_transform.position += Vec3 { 0, keyboard[SDL_SCANCODE_LSHIFT] ? -1.0f / 500 : 0, 0 } * SPEED;

	player_transform.position += (keyboard[SDL_SCANCODE_W] ? facing * (1.0f / 500) : Vec3{}) * SPEED;
	player_transform.position += (keyboard[SDL_SCANCODE_S] ? facing * (-1.0f / 500) : Vec3{}) * SPEED;

	player_transform.position += (keyboard[SDL_SCANCODE_D] ? right * (1.0f / 500) : Vec3{}) * SPEED;
	player_transform.position += (keyboard[SDL_SCANCODE_A] ? right * (-1.0f / 500) : Vec3{}) * SPEED;

	return false;
}

void physics_system(State& ecs, Application& app) {
	auto [ transforms, physics ] = ecs.components<Transform, PhysicsBody>();
	const auto& entities = ecs.entities(physics_system);

	for (auto& entity : entities) {
		transforms[entity.index].position += Vec3{ 0, -1, 0 } * physics[entity.index].speed;
	}
}

void render_system(State& ecs, Application& app) {
	glViewport(0, 0, app.width, app.height);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto [ transforms, meshes, cameras ] = ecs.components<Transform, MeshRenderer, Camera>();
	const auto& entities = ecs.entities(render_system);

	Camera& player_camera { cameras[app.player.index] };
	Transform& player_transform { transforms[app.player.index] };

	GLint u_proj_loc { glGetUniformLocation(app.shader, "u_proj") };
	GLint u_color_loc { glGetUniformLocation(app.shader, "u_color") };

	Mat4x4 cam { view(player_transform.position, player_camera.facing, player_camera.up) };
	Mat4x4 persp { perspective(static_cast<float>(app.width) / app.height, player_camera.fov, player_camera.near, player_camera.far) };

	for (auto& entity : entities) {
		Mat4x4 model { transforms[entity.index].model() };
		Mat4x4 proj { persp * cam * model };
		glUniformMatrix4fv(u_proj_loc, 1, GL_FALSE, &proj);

		MeshRenderer& mesh_renderer { meshes[entity.index] };
		glUniform3fv(u_color_loc, 1, &(mesh_renderer.color));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_renderer.mesh->ebo);
		glDrawElements(GL_TRIANGLES, mesh_renderer.mesh->num_indices, GL_UNSIGNED_INT, 0);
	}

	SDL_GL_SwapWindow(app.window);
}