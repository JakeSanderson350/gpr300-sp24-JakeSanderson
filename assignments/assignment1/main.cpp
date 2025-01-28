#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>

#include <ew/model.h>
#include <ew/transform.h>

#include <ew/texture.h>

#include <ew/camera.h>
#include <ew/cameracontroller.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

//Cached data

ew::Camera camera;
ew::CameraController camerController;

struct FullscreenQuad
{
	GLuint vao;
	GLuint vbo;
} fullscreenQuad;

static float quadVertices[] = {
	// pos (x,y) texcoord (u,v)
	-1.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
};

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

struct FrameBuffer
{
	GLuint fbo;
	GLuint color0;
	GLuint color1;
	GLuint depth;
} framebuffer;

void initCamera()
{
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
}

void initFullscreenQuad()
{
	//initialize fullscreen quad
	glGenVertexArrays(1, &fullscreenQuad.vao);
	glGenBuffers(1, &fullscreenQuad.vbo);

	glBindVertexArray(fullscreenQuad.vao);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuad.vbo);

	//glBindVertexBuffer(0, fullscreenQuad.vbo, 0, sizeof(quadVertices));
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	//glBindVertexArray(0);

	glEnableVertexAttribArray(0); // positions
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
	glEnableVertexAttribArray(1); // texcoords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*) (2 * sizeof(float)));
}

void initFrameBuffer()
{
	glGenFramebuffers(1, &framebuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	// color attachment
	glGenTextures(1, &framebuffer.color0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);

	//check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer incomplete: %d", 1);
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawThing(ew::Shader& _shader, ew::Model& _model, ew::Transform& _modelTranform, GLuint& _texture)
{
	// Bind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// 1. pipeline definition
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);

	// 2. gfx pass
	/*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);*/

	_modelTranform.rotation = glm::rotate(_modelTranform.rotation, deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));

	_shader.use();
	_shader.setInt("_MainTexture", 0);
	_shader.setMat4("_TransformModel", _modelTranform.modelMatrix());
	_shader.setMat4("_CameraViewproj", camera.projectionMatrix() * camera.viewMatrix());
	//Material uniforms
	_shader.setFloat("_Material.Ka", material.Ka);
	_shader.setFloat("_Material.Kd", material.Kd);
	_shader.setFloat("_Material.Ks", material.Ks);
	_shader.setFloat("_Material.Shininess", material.Shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture);

	_model.draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawFrameBuffer(ew::Shader& _shader)
{
	glDisable(GL_DEPTH_TEST);

	_shader.use();
	_shader.setInt("_MainTexture", 0);

	glBindVertexArray(fullscreenQuad.vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color0);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//Set up assets
	ew::Shader lit_shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader full_shader = ew::Shader("assets/fullscreen.vert", "assets/fullscreen.frag");
	ew::Shader inverse_shader = ew::Shader("assets/inverse.vert", "assets/inverse.frag");
	ew::Shader grayscale_shader = ew::Shader("assets/grayscale.vert", "assets/grayscale.frag");
	ew::Shader blur_shader = ew::Shader("assets/blur.vert", "assets/blur.frag");
	ew::Shader chromatic_shader = ew::Shader("assets/chromatic.vert", "assets/chromatic.frag");

	ew::Model suzanne = ew::Model("assets/suzanne.obj");
	ew::Transform monkeyTransform;

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	//Set up camera
	initCamera();

	initFullscreenQuad();

	initFrameBuffer();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		// Clear default buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);

		//MOVE CAMERA
		camerController.move(window, &camera, deltaTime);

		//RENDER
		drawThing(lit_shader, suzanne, monkeyTransform, brickTexture);
		drawFrameBuffer(chromatic_shader);

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");

	if (ImGui::Button("Reset Camera"))
	{
		initCamera();
	}
	
	if (ImGui::CollapsingHeader("Material"))
	{
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}

	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color0, ImVec2(screenWidth, screenHeight));

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

