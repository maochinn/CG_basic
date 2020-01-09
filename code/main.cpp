#if defined(NANOGUI_GLAD)
#if defined(NANOGUI_SHARED) && !defined(GLAD_GLAPI_EXPORT)
#define GLAD_GLAPI_EXPORT
#endif

#include <glad/glad.h>
#else
#if defined(__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#else
#define GL_GLEXT_PROTOTYPES
#endif
#endif

#include <glad\glad.h>
#include <GLFW/glfw3.h>

#include <nanogui/nanogui.h>
#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Cube.h"
#include "Sphere.h"
#include "Texture.h"
//#include "ObjModel.h"
#include "ShadowMap.h"
#include "DeferredShading.h"
#include "Model.h"

GLfloat last_x = 400, last_y = 300;
bool firstMouse = true;
bool mouse_press = false;
bool keys[1024] = { 0 };

int WIDTH = 800;	//width of window
int HEIGHT = 800;	//height of window

const GLuint NR_LIGHTS = 32;

GLFWwindow* window = nullptr;
nanogui::Screen *screen = nullptr;
Camera* camera = nullptr;
Cube* cube = nullptr;
Cube* texture_cube;
Sphere* ball = nullptr;
Texture2D* texture = nullptr;
Texture2D* texture2 = nullptr;
UBO* matrix = nullptr;
UBO* light = nullptr;
UBO* view = nullptr;
//ObjModel* model = nullptr;
Model* nanosuit = nullptr;
//ShadowMap* shadow_map = nullptr;
DeferredShading* deferred_shading = nullptr;


std::vector<glm::vec3> light_positions;
std::vector<glm::vec3> light_colors;

void initialize();
void setCallBack();
void setGUI();
void setUBO();
void setKeyboard(float dt);
void renderSceneDepth();
void renderScene();

int main(int /* argc */, char ** /* argv */) {

	glfwInit();

	glfwSetTime(0);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 0);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Create a GLFWwindow object
	window = glfwCreateWindow(WIDTH, HEIGHT, "Basic", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);


	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Could not initialize GLAD!");
	glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM

	glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	glfwGetFramebufferSize(window, &WIDTH, &HEIGHT);
	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSwapInterval(0);
	glfwSwapBuffers(window);

	
	initialize();
	setCallBack();
	setGUI();

	GLfloat current_frame = 0.0f;
	GLfloat last_frame = 0.0f;
	GLfloat delta_time = 0.0f;


	std::vector<glm::vec3> objectPositions;
	objectPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(-3.0, -3.0, 3.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 3.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, 3.0));

	// Game loop
	while (!glfwWindowShouldClose(window)) {
		// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		glfwGetFramebufferSize(window, &WIDTH, &HEIGHT);

		current_frame = (float)glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		setKeyboard(delta_time);


		////render depth map to shadow map
		//{
		//	shadow_map->setLight(glm::vec3(5.0f, 5.0f, 0.0f), glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(10.0f, 10.0f));

		//	shadow_map->bindFBO();
		//	glViewport(0, 0, shadow_map->resolution.x, shadow_map->resolution.y);
		//	// Clear all relevant buffers
		//	//glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
		//	glClear(GL_DEPTH_BUFFER_BIT);

		//	//set UBO
		//	glm::mat4 view = shadow_map->getViewMtx();
		//	glm::mat4 projection = shadow_map->getProjectionMtx();
		//	glBindBuffer(GL_UNIFORM_BUFFER, matrix->ubo);
		//	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection[0][0]);
		//	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view[0][0]);
		//	glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//	renderSceneDepth();
		//}

		// 1. Geometry Pass: render scene's geometry/color data into gbuffer
		deferred_shading->bindFBO();
		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(0.2f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		setUBO();
		
		deferred_shading->shader_geometry_pass.Use();

		glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/0, matrix->ubo, 0, matrix->size);
		glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/1, light->ubo, 0, light->size);
		glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/2, view->ubo, 0, view->size);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);


		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (glm::vec3 pos : objectPositions)
		{
			glm::mat4 model_matrix = glm::mat4();
			model_matrix = glm::translate(model_matrix, pos);
			model_matrix = glm::scale(model_matrix, glm::vec3(0.5f, 0.5f, 0.5f));

			glUniformMatrix4fv(glGetUniformLocation(deferred_shading->shader_geometry_pass.Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);
			nanosuit->Draw(deferred_shading->shader_geometry_pass);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		


		// 2. Lighting Pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
		//render
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		setUBO();
		renderScene();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, deferred_shading->fbo.buffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // 写入到默认帧缓冲
		glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		for (GLuint i = 0; i < NR_LIGHTS; i++)
		{
			cube->render(light_positions[i], light_colors[i]);
		}

		// Draw nanogui
		screen->drawContents();
		screen->drawWidgets();

		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();

	return 0;
}

void initialize()
{
	//ubo
	//matrices for 3D persective projection of uniform buffer
	matrix = new UBO();
	matrix->size = 2 * sizeof(glm::mat4);
	glGenBuffers(1, &matrix->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, matrix->ubo);
	glBufferData(GL_UNIFORM_BUFFER, matrix->size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	light = new UBO();
	//light->size = 4 * sizeof(glm::vec4);
	light->size = 4 * sizeof(glm::vec4) + NR_LIGHTS * 5 * sizeof(glm::vec4);
	glGenBuffers(1, &light->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, light->ubo);
	glBufferData(GL_UNIFORM_BUFFER, light->size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	view = new UBO();
	view->size = 2 * sizeof(glm::vec4);
	glGenBuffers(1, &view->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, view->ubo);
	glBufferData(GL_UNIFORM_BUFFER, view->size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Create a nanogui screen and pass the glfw pointer to initialize
	screen = new nanogui::Screen();
	screen->initialize(window, true);
	
	Shader shader("code/shaders/noTexture.vert", "code/shaders/noTexture.frag");
	Shader t_shader("code/shaders/singleTexture.vert", "code/shaders/singleTexture.frag");
	Shader simpleDepthShader("code/shaders/simpleDepth.vert", "code/shaders/simpleDepth.frag");
	Shader shadowMap("code/shaders/shadowMap.vert", "code/shaders/shadowMap.frag");

	texture = new Texture2D("img/kurumi.png");
	texture2 = new Texture2D("img/chara_kurumi.png");

	cube = new Cube(shader);
	texture_cube = new Cube(t_shader);
	ball = new Sphere(shadowMap);
	camera = new Camera();

	//model = new ObjModel(shadowMap, "model/track.obj", "object");

	//shadow_map = new ShadowMap(simpleDepthShader, shadowMap, glm::ivec2(512, 512));

	//deferred_shading = new DeferredShading(
	//	Shader("code/shaders/g_buffer.vert", "code/shaders/g_buffer.frag"),
	//	Shader("code/shaders/deferredShading.vert", "code/shaders/deferredShading.frag"),
	//	glm::ivec2(WIDTH, HEIGHT));

	deferred_shading = new DeferredShading(
		Shader(
			"code/shaders/g_buffer.vert",
			"code/shaders/g_buffer.tesc",
			"code/shaders/g_buffer.tese",
			nullptr,
			"code/shaders/g_buffer.frag"),
		Shader("code/shaders/deferredShading.vert", "code/shaders/deferredShading.frag"),
		glm::ivec2(WIDTH, HEIGHT));

	nanosuit = new Model("model/nanosuit/nanosuit.obj");

	// - Colors
	srand(13);
	for (GLuint i = 0; i < NR_LIGHTS; i++)
	{
		// Calculate slightly random offsets
		GLfloat xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		GLfloat yPos = ((rand() % 100) / 100.0) * 6.0;
		GLfloat zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		light_positions.push_back(glm::vec3(xPos, yPos, zPos));
		// Also calculate random color
		GLfloat rColor = ((rand() % 100) / 100.0f);
		GLfloat gColor = ((rand() % 100) / 100.0f);
		GLfloat bColor = ((rand() % 100) / 100.0f);
		light_colors.push_back(glm::vec3(rColor, gColor, bColor));
	}
}
void setGUI()
{
	// Create nanogui gui
	nanogui::Window *setting_window = new nanogui::Window(screen, "float window");
	setting_window->setPosition(nanogui::Vector2i(15, 15));
	setting_window->setLayout(new nanogui::GroupLayout());

	/* No need to store a pointer, the data structure will be automatically
	freed when the parent window is deleted */
	new nanogui::Label(setting_window, "test label", "sans-bold");


	screen->setVisible(true);
	screen->performLayout();
}
void setCallBack()
{
	glfwSetCursorPosCallback(window,
		[](GLFWwindow *, double x_pos, double y_pos) {
		screen->cursorPosCallbackEvent(x_pos, y_pos);
		if (mouse_press)
		{
			if (firstMouse)
			{
				last_x = (float)x_pos;
				last_y = (float)y_pos;
				firstMouse = false;
			}

			GLfloat xoffset = (float)x_pos - last_x;
			GLfloat yoffset = last_y - (float)y_pos;  // Reversed since y-coordinates go from bottom to left

			camera->ProcessMouseMovement(-xoffset, -yoffset);
		}

		last_x = (float)x_pos;
		last_y = (float)y_pos;
	}
	);

	glfwSetMouseButtonCallback(window,
		[](GLFWwindow *, int button, int action, int modifiers)
	{
		screen->mouseButtonCallbackEvent(button, action, modifiers);
		screen->mouseButtonCallbackEvent(button, action, modifiers);
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
			mouse_press = true;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
			mouse_press = false;
	}
	);

	glfwSetKeyCallback(window,
		[](GLFWwindow *, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		if (key >= 0 && key < 1024)
		{
			if (action == GLFW_PRESS)
				keys[key] = true;
			else if (action == GLFW_RELEASE)
				keys[key] = false;
		}
		screen->keyCallbackEvent(key, scancode, action, mods);
	}
	);

	glfwSetCharCallback(window,
		[](GLFWwindow *, unsigned int codepoint) {
		screen->charCallbackEvent(codepoint);
	}
	);

	glfwSetDropCallback(window,
		[](GLFWwindow *, int count, const char **filenames)
	{
		screen->dropCallbackEvent(count, filenames);
	}
	);

	glfwSetScrollCallback(window,
		[](GLFWwindow *, double x, double y)
	{
		screen->scrollCallbackEvent(x, y);
	});

	glfwSetFramebufferSizeCallback(window,
		[](GLFWwindow *, int width, int height) {
		screen->resizeCallbackEvent(width, height);
	}
	);
}
void setUBO()
{
	//update ubo_perspective
	glm::mat4 view_matrix = camera->GetViewMatrix();
	glm::mat4 projection_matrix;
	projection_matrix = glm::perspective(glm::radians(camera->Zoom), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.01f, 1000.0f);
	glBindBuffer(GL_UNIFORM_BUFFER, matrix->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection_matrix[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view_matrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, light->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), &camera->Front[0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(glm::vec4), &glm::vec3(0.25f, 0.25f, 0.25f)[0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::vec4), sizeof(glm::vec4), &glm::vec3(0.3f, 0.3f, 0.3f)[0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::vec4), sizeof(glm::vec4), &glm::vec3(0.1f, 0.1f, 0.1f)[0]);
	
	float constant = 1.0f;
	float linear = 0.7f;
	float quadratic = 1.8f;
	glm::vec3 attenuation(constant, linear, quadratic);
	for (GLuint i = 0; i < NR_LIGHTS; i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, (4 + i * 5) * sizeof(glm::vec4), sizeof(glm::vec4), &light_positions[i][0]);
		glBufferSubData(GL_UNIFORM_BUFFER, (5 + i * 5) * sizeof(glm::vec4), sizeof(glm::vec4), &light_colors[i][0]);
		glBufferSubData(GL_UNIFORM_BUFFER, (6 + i * 5) * sizeof(glm::vec4), sizeof(glm::vec4), &light_colors[i][0]);
		glBufferSubData(GL_UNIFORM_BUFFER, (7 + i * 5) * sizeof(glm::vec4), sizeof(glm::vec4), &light_colors[i][0]);
		glBufferSubData(GL_UNIFORM_BUFFER, (8 + i * 5) * sizeof(glm::vec4), sizeof(glm::vec4), &attenuation[0]);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, view->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), &camera->Front[0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(glm::vec4), &camera->Position[0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}
void renderSceneDepth()
{
	//glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/0, matrix->ubo, 0, matrix->size);

	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	//glDisable(GL_BLEND);

	////cube->render(glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(1.0f), &shadow_map->shader);
	//ball->render(glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(1.0f), &shadow_map->depth_shader);
	//model->render(glm::mat4(), &shadow_map->depth_shader);
	//{
	//	glm::mat4 model;
	//	model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
	//	model = glm::scale(model, glm::vec3(10.0f, 1.0f, 10.0f));
	//	cube->render(model, glm::vec3(1.0f), &shadow_map->depth_shader);
	//}

	//texture_cube->render(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(1.0f), &shadow_map->depth_shader);
}
void renderScene()
{
	glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/0, matrix->ubo, 0, matrix->size);
	glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/1, light->ubo, 0, light->size);
	glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/2, view->ubo, 0, view->size);

	glEnable(GL_DEPTH_TEST);

	//glm::mat4 model;
	//model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
	//model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
	//cube->render(model);
	////cube->shader.Use();
	//glUniformMatrix4fv(glGetUniformLocation(cube->shader.Program, "u_model"), 1, GL_FALSE, &model[0][0]);
	//nanosuit->Draw(cube->shader);

	deferred_shading->render();

	//shadow_map->shadow_shader.Use();
	//shadow_map->bind(0);
	//glUniform1i(glGetUniformLocation(shadow_map->shadow_shader.Program, "u_shadow_map"), 0);
	//glUniformMatrix4fv(glGetUniformLocation(shadow_map->shadow_shader.Program, "u_light_space_matrix"), 1, GL_FALSE, &shadow_map->getViewProjectionMtx()[0][0]);
	//
	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	//glDisable(GL_BLEND);

	////cube->render(glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), &shadow_map->shadow_shader);
	//ball->render(glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), &shadow_map->shadow_shader);
	//model->render(glm::mat4(), &shadow_map->shadow_shader);

	//{
	//	glm::mat4 model;
	//	model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
	//	model = glm::scale(model, glm::vec3(10.0f, 1.0f, 10.0f));
	//	cube->render(model, glm::vec3(1.0f), &shadow_map->shadow_shader);
	//}

	//texture_cube->shader.Use();
	//shadow_map->bind(0);
	//glUniform1i(glGetUniformLocation(texture_cube->shader.Program, "u_material.t_diffuse"), 0);
	//shadow_map->bind(1);
	//glUniform1i(glGetUniformLocation(texture_cube->shader.Program, "u_material.t_specular"), 1);
	//texture_cube->render(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	////Texture2D::unbind(0);
	////Texture2D::unbind(1);
}
void setKeyboard(/*delta time*/float dt)
{
	if (keys[GLFW_KEY_W])
		camera->ProcessKeyboard(FORWARD, dt);
	if (keys[GLFW_KEY_S])
		camera->ProcessKeyboard(BACKWARD, dt);
	if (keys[GLFW_KEY_A])
		camera->ProcessKeyboard(LEFT, dt);
	if (keys[GLFW_KEY_D])
		camera->ProcessKeyboard(RIGHT, dt);

}