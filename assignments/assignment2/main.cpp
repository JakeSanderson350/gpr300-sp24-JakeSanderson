#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "glm/gtx/transform.hpp"


#include <ew/procGen.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

static glm::vec4 lightOrbitRad = { 2.0f, 2.0f, -2.0f, 1.0f };

//Cached data
#include <ew/shader.h>

#include <ew/model.h>
#include <ew/transform.h>

#include <ew/texture.h>

#include <ew/camera.h>
#include <ew/cameracontroller.h>
ew::Camera camera;
ew::CameraController camerController;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

struct Light {
	glm::vec3 lightPos;
	glm::vec3 lightColor;
}light;

glm::vec3 suzzanePos = glm::vec3(0.0f);

float minBias = 0.005;
float maxBias = 0.05;

struct DepthBuffer {
	GLuint fbo;
	GLuint depth;

	void Initialize()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// Depth attachment
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		/*GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);*/

		//check completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("Framebuffer incomplete: %d", 1);
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}depthBuffer;

void initCamera()
{
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
}

void updateScene(float deltaTime)
{
	const auto rym = glm::rotate(/*(float)time.absolute*/deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
	light.lightPos = rym * lightOrbitRad;
}

void drawThing(ew::Shader& _shader, ew::Shader& _shaderShadow, ew::Model& _model, ew::Transform& _modelTranform, GLuint& _texture, ew::Mesh& _plane)
{
	const auto lightProj = glm::ortho(-10.f, 10.f, -10.f, 10.0f, 0.1f, 100.f);
	const auto lightView = glm::lookAt(light.lightPos, glm::vec3(0.0f), glm::vec3(0.0, -1.0f, 0.0f));
	const auto lightViewProj = lightProj * lightView;

	glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer.fbo);
	{
		glViewport(0, 0, 1024, 1024);

		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		_shaderShadow.use();

		_shaderShadow.setMat4("_TransformModel", _modelTranform.modelMatrix());
		_shaderShadow.setMat4("_LightViewproj", lightViewProj);

		_model.draw();
		glCullFace(GL_BACK);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render lighting
	{
		// 1. pipeline definition
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);

		glViewport(0, 0, screenWidth, screenHeight);

		// 2. gfx pass
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);

		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _texture);*/

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthBuffer.depth);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//_modelTranform.rotation = glm::rotate(_modelTranform.rotation, deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));

		_shader.use();
		_shader.setInt("_ShadowMap", 0);
		_shader.setMat4("_TransformModel", _modelTranform.modelMatrix());
		_shader.setMat4("_CameraViewproj", camera.projectionMatrix() * camera.viewMatrix());
		_shader.setMat4("_LightViewproj", lightViewProj);
		//Material uniforms
		_shader.setFloat("_Material.Ka", material.Ka);
		_shader.setFloat("_Material.Kd", material.Kd);
		_shader.setFloat("_Material.Ks", material.Ks);
		_shader.setFloat("_Material.Shininess", material.Shininess);

		_shader.setVec3("_CamPos", camera.position);
		_shader.setVec3("_Light.lightPos", light.lightPos);
		_shader.setVec3("_Light.lightColor", light.lightColor);
		_shader.setFloat("_MinBias", minBias);
		_shader.setFloat("_MaxBias", maxBias);


		_model.draw();

		_shader.setMat4("_TransformModel", glm::translate(glm::vec3(0.0f, -2.0f, 0.0f)));
		_plane.draw();
	}
}

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//Set up assets
	ew::Shader lit_shader = ew::Shader("assets/litNew.vert", "assets/litNew.frag");
	ew::Shader shadow_shader = ew::Shader("assets/shadowpass.vert", "assets/shadowpass.frag");

	ew::Model suzanne = ew::Model("assets/suzanne.obj");
	ew::Transform monkeyTransform;

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	ew::Mesh plane = ew::createPlane(20, 20, 10);

	//Set up camera
	initCamera();

	depthBuffer.Initialize();
	light.lightPos = glm::vec3(0.0, 2.0, 0.0);
	light.lightColor = glm::vec3(1.0, 0.0, 1.0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//MOVE CAMERA
		camerController.move(window, &camera, deltaTime);

		updateScene(time);

		//RENDER
		monkeyTransform.position = suzzanePos;
		drawThing(lit_shader, shadow_shader, suzanne, monkeyTransform, brickTexture, plane);

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

	ImGui::DragFloat3("Suzanne Pos", &suzzanePos[0], 0.25f, -10.0f, 10.0f);
	
	if (ImGui::CollapsingHeader("Material"))
	{
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}

	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::DragFloat3("Light Pos", &light.lightPos[0], -10.0, 10.0);
		ImGui::ColorEdit3("Light Color", &light.lightColor[0]);
		ImGui::SliderFloat("Min Shadow Bias", &minBias, 0.0f, 0.05f);
		ImGui::SliderFloat("Max Shadow Bias", &maxBias, 0.0f, 0.15f);
	}

	ImGui::Image((ImTextureID)(intptr_t)depthBuffer.depth, ImVec2(256, 256));

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

