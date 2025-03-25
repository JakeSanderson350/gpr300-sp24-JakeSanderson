#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/procGen.h>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
int fbWidth = 1080;
int fbHeight = 720;
float prevFrameTime;
float deltaTime;

//Cached data
#include <ew/shader.h>

#include <ew/model.h>
#include <ew/transform.h>

#include <ew/texture.h>

#include <ew/camera.h>
#include <ew/cameracontroller.h>
ew::Camera camera;
ew::CameraController camerController;

static float quadVertices[] = {
	// pos (x,y) texcoord (u,v)
	-1.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
};

struct FullscreenQuad
{
	GLuint vao;
	GLuint vbo;

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
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(2 * sizeof(float)));
	}
} fullscreenQuad;

struct FrameBuffer
{
	GLuint fbo;
	GLuint color0;
	GLuint color1;
	GLuint color2;
	GLuint lights;
	GLuint depth;

	void Initialize()
	{
		glGenFramebuffers(1, &framebuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

		// color attachment
		glGenTextures(1, &framebuffer.color0); // albedo
		glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, fbWidth, fbHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);

		// Other color attach for HDR
		glGenTextures(1, &framebuffer.color1);	// position
		glBindTexture(GL_TEXTURE_2D, framebuffer.color1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, fbWidth, fbHeight, 0, GL_RGBA, GL_FLOAT, nullptr); //Changed from GL_RGB to GL_RGB16F
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebuffer.color1, 0);

		// Other color attach for HDR
		glGenTextures(1, &framebuffer.color2);	// normal
		glBindTexture(GL_TEXTURE_2D, framebuffer.color2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, fbWidth, fbHeight, 0, GL_RGBA, GL_FLOAT, nullptr); //Changed from GL_RGB to GL_RGB16F
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, framebuffer.color2, 0);

		// Other color attach for Lights
		glGenTextures(1, &framebuffer.lights);	// deffered lights
		glBindTexture(GL_TEXTURE_2D, framebuffer.lights);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, fbWidth, fbHeight, 0, GL_RGBA, GL_FLOAT, nullptr); //Changed from GL_RGB to GL_RGB16F
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, framebuffer.lights, 0);

		// Depth attachment
		glGenTextures(1, &framebuffer.depth);
		glBindTexture(GL_TEXTURE_2D, framebuffer.depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, fbWidth, fbHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depth, 0);

		GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, attachments);

		//check completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("Framebuffer incomplete: %d", 1);
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
} framebuffer;

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

const int lightEdgeNum = 8;
const int lightsNum = lightEdgeNum * lightEdgeNum;
Light lights[lightsNum];

float spacer = 3;
int suzaneNum = 1;

ew::Mesh sphere;

void initCamera()
{
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
}

glm::vec3 randomColor()
{
	float r = (rand() % 256) / 255.0f;
	float g = (rand() % 256) / 255.0f;
	float b = (rand() % 256) / 255.0f;

	return glm::vec3(r, g, b);
}

void initLights()
{
	for (int i = 0; i < lightEdgeNum; i++)
	{
		for (int j = 0; j < lightEdgeNum; j++)
		{
			lights[(i * lightEdgeNum) + j].lightPos = glm::vec3(i * spacer, 4, j * spacer);
			lights[(i * lightEdgeNum) + j].lightColor = randomColor();
		}
	}

	/*for (int i = 0; i < lightsNum)
	{

	}*/
}

void drawLights(ew::Shader& _shader)
{

	/*glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);*/
	//glBlitNamedFramebuffer(framebuffer.fbo, framebuffer.lights, 0, 0, fbWidth, fbHeight, 0, 0, screenWidth, screenWidth, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	glEnable(GL_DEPTH_TEST);

	_shader.use();

	// Uniforms
	_shader.setMat4("_CameraViewproj", camera.projectionMatrix() * camera.viewMatrix());

	// Draw lights
	int lightsSize = sizeof(lights) / sizeof(lights[0]);
	for (int i = 0; i < 64; i++)
	{
		_shader.setMat4("_TransformModel", glm::translate(lights[i].lightPos));
		_shader.setVec3("_Light.lightPos", lights[i].lightPos);
		_shader.setVec3("_Light.lightColor", lights[i].lightColor);
		sphere.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawLighting(ew::Shader& _shader)
{
	glDisable(GL_DEPTH_TEST);

	_shader.use();

	// set buffers
	_shader.setInt("_Albedo", 0);
	_shader.setInt("_Position", 1);
	_shader.setInt("_Normal", 2);

	// Uniforms
	_shader.setFloat("_Material.Ka", material.Ka);
	_shader.setFloat("_Material.Kd", material.Kd);
	_shader.setFloat("_Material.Ks", material.Ks);
	_shader.setFloat("_Material.Shininess", material.Shininess);
	_shader.setVec3("_CamPos", camera.position);
	/*_shader.setVec3("_Light.lightPos", light.lightPos);
	_shader.setVec3("_Light.lightColor", light.lightColor);*/
	for (int i = 0; i < lightsNum; i++)
	{
		_shader.setVec3("_Lights[" + std::to_string(i) + "].lightPos", light.lightPos);
		_shader.setVec3("_Lights[" + std::to_string(i) + "].lightColor", light.lightColor);
	}

	glBindVertexArray(fullscreenQuad.vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color2);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void drawGeometry(ew::Shader& _shader, ew::Model& _model, ew::Transform& _modelTransform, GLuint& _texture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	// bind framebuffer here/
	// 1. pipeline definition
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);

	// 2. gfx pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);

	/*glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture);*/

	//_modelTransform.rotation = glm::rotate(_modelTransform.rotation, deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));

	_shader.use();
	//_shader.setInt("_MainTexture", 0);
	_shader.setMat4("_CameraViewproj", camera.projectionMatrix() * camera.viewMatrix());

	for (int i = -suzaneNum; i <= suzaneNum; i++)
	{
		for (int j = -suzaneNum; j <= suzaneNum; j++)
		{
			// Draw suzanne
			//_modelTransform.position = glm::vec3(_modelTransform.position.x + (i * 2), 0, _modelTransform.position.z + (j * 2));
			_shader.setMat4("_TransformModel", glm::translate(_modelTransform.modelMatrix(), glm::vec3(i * spacer, 0, j * spacer)));
			_model.draw();
		}
	}

	//unbind here
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//Set up assets
	ew::Shader geo_shader = ew::Shader("assets/geometry.vert", "assets/geometry.frag");
	ew::Shader lit_shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader lights_shader = ew::Shader("assets/lights.vert", "assets/lights.frag");

	ew::Model suzanne = ew::Model("assets/suzanne.obj");
	ew::Transform monkeyTransform;

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	
	fullscreenQuad.initFullscreenQuad();
	framebuffer.Initialize();
	initLights();

	light.lightPos = glm::vec3(0.0, 2.0, 0.0);
	light.lightColor = glm::vec3(1.0, 0.0, 1.0);
	sphere.load(ew::createSphere(0.5f, 20));

	//Set up camera
	initCamera();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//MOVE CAMERA
		camerController.move(window, &camera, deltaTime);

		//RENDER
		drawGeometry(geo_shader, suzanne, monkeyTransform, brickTexture);
		drawLights(lights_shader);
		drawLighting(lit_shader);
		

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

	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::DragFloat3("Light Pos", &light.lightPos[0], 1.0f, -100.0f, 100.0f);
		ImGui::ColorEdit3("Light Color", &light.lightColor[0]);
	}

	ImGui::SliderInt("Num Suzes", &suzaneNum, 1, 100);
	ImGui::SliderFloat("Spacer", &spacer, 0.0f, 10.0f);
	int totalSuzes = pow(2 * suzaneNum + 1, 2);
	ImGui::Text("Number of suzes: ");
	ImGui::Text(std::to_string(totalSuzes).c_str());

	ImGui::Image((ImTextureID)(intptr_t)framebuffer.lights, ImVec2(fbWidth, fbHeight));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color1, ImVec2(fbWidth, fbHeight));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color2, ImVec2(fbWidth, fbHeight));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.depth, ImVec2(fbWidth, fbHeight));


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

