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

#include "js/portal.h"

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
	GLuint stencil;

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

		GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);

		// Depth attachment
		glGenTextures(1, &framebuffer.depth);
		glBindTexture(GL_TEXTURE_2D, framebuffer.depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, fbWidth, fbHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depth, 0);

		// Stencil
		/*glGenTextures(1, &framebuffer.stencil);
		glBindTexture(GL_TEXTURE_2D, framebuffer.stencil);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, fbWidth, fbHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, framebuffer.stencil, 0);*/

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
	float radius;
}light;

const int lightEdgeNum = 2;
const int lightsNum = lightEdgeNum * lightEdgeNum;
Light lights[lightsNum];

float lightSpacer = 8;
float suzanneSpacer = 3;
int suzaneNum = 1;

glm::vec3 p1Position = glm::vec3(0,1,0);
glm::vec3 p2Position = glm::vec3(-5, 1, 0);
glm::vec3 p1Rotation = glm::vec3(0, 0, 0);
glm::vec3 p2Rotation = glm::vec3(0, 0, 0);

//Global models
ew::Model* suzanne;
ew::Transform monkeyTransform;
GLuint brickTexture;

ew::Mesh sphere;
ew::Mesh plane;
ew::Transform planeTransform;

js::Portal* portal1 = nullptr;
js::Portal* portal2 = nullptr;
std::vector<js::Portal*> portals;

void Cleanup()
{
	delete portal1;
	delete portal2;
	delete suzanne;
}

void initCamera()
{
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
}

void initModels()
{
	sphere.load(ew::createSphere(0.5f, 20));
	plane = ew::createPlane(50, 50, 10);

	monkeyTransform.position = glm::vec3(0, 0, 3);
	planeTransform.position = glm::vec3(0, -3, 0);
}

void initPortals()
{
	portal1 = new js::Portal();
	portal2 = new js::Portal(portal1);
	portal1->SetDestination(portal2);

	portal1->initBuffers();
	portal2->initBuffers();

	portal1->SetColor(glm::vec3(0.0f, 1.0f, 0.0f));
	portal2->SetColor(glm::vec3(0.0f, 0.0f, 1.0f));
	portal1->transform.position = p1Position;
	portal1->transform.rotation = glm::quat(glm::radians(p1Rotation));;
	portal2->transform.position = p2Position;
	portal2->transform.rotation = glm::quat(glm::radians(p2Rotation));

	portals.push_back(portal1);
	portals.push_back(portal2);
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
			lights[(i * lightEdgeNum) + j].lightPos = glm::vec3(i * lightSpacer, 4, j * lightSpacer);
			lights[(i * lightEdgeNum) + j].lightColor = randomColor();
			lights[(i * lightEdgeNum) + j].radius = 45.0f;
		}
	}

	// Move lights os they are more centered over plane
	for (int i = 0; i < lightsNum; i++)
	{
		lights[i].lightPos += glm::vec3(-8, 0, -8);
	}
}

void drawOtherObjects(glm::mat4 const& viewMat, glm::mat4 const& projMat, ew::Shader geoShader)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brickTexture);

	geoShader.use();
	geoShader.setInt("_MainTexture", 0);
	geoShader.setMat4("_CameraViewproj", projMat * viewMat);	

	geoShader.setMat4("_TransformModel", monkeyTransform.modelMatrix());
	suzanne->draw();
	geoShader.setMat4("_TransformModel", planeTransform.modelMatrix());
	plane.draw();
	

	/*geoShader.setMat4("_TransformModel", glm::translate(portal1->transform.position));
	sphere.draw();
	geoShader.setMat4("_TransformModel", glm::translate(portal2->transform.position));
	sphere.draw();*/

}

void recursiveDraw(glm::mat4 const& viewMat, glm::mat4 const& projMat, size_t maxRecursionLevel, size_t recursionLevel, ew::Shader portalShader, ew::Shader geoShader)
{
	for (js::Portal* portal : portals)
	{
		// Disable color and depth writing
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		// Disable depth test
		glDisable(GL_DEPTH_TEST);

		// Enable stencil test, not to draw
		// outside of current portal depth
		glEnable(GL_STENCIL_TEST);

		// Fail stencil test when inside of outer portal
		// (fail where we should be drawing the inner portal)
		glStencilFunc(GL_ALWAYS, recursionLevel + 1, 0xFF);

		// Increment stencil value on stencil fail
		// (on area of inner portal)
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		// Enable (writing into) all stencil bits
		glStencilMask(0xFF);

		// Draw portal into stencil buffer
		portal->draw(viewMat, projMat, portalShader);

		// Calculate viewing matrix of virtual cam
		glm::mat4 destinationView =
			viewMat * portal->transform.modelMatrix()
			* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f) * portal->transform.rotation)
			* glm::inverse(portal->GetDestination()->transform.modelMatrix());

		// Base case, render inside of inner portal
		if (recursionLevel == maxRecursionLevel)
		{
			// Enable color and depth drawing
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask(GL_TRUE);

			// Clear the depth buffer so we don't interfere with stuff
			// outside of this inner portal
			glClear(GL_DEPTH_BUFFER_BIT);

			// Enable the depth test
			// So the stuff we render here is rendered correctly
			glEnable(GL_DEPTH_TEST);

			// Enable stencil test
			// So we can limit drawing inside of the inner portal
			glEnable(GL_STENCIL_TEST);

			// Disable drawing into stencil buffer
			glStencilMask(0x00);

			// Draw only where stencil value == recursionLevel + 1
			// which is where we just drew the new portal
			glStencilFunc(GL_EQUAL, recursionLevel + 1, 0xFF);

			// Draw scene objects with destinationView, limited to stencil buffer
			// use an edited projection matrix to set the near plane to the portal plane
			drawOtherObjects(destinationView, portal->ClippedProjMat(destinationView, projMat, camera), geoShader);
		}
		else
		{
			// Recursion case
			// Pass our new view matrix and the clipped projection matrix (see above)
			recursiveDraw(destinationView, portal->ClippedProjMat(destinationView, projMat, camera), maxRecursionLevel, recursionLevel + 1, portalShader, geoShader);
		}

		// Disable color and depth drawing
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		// Enable stencil test and stencil drawing
		glEnable(GL_STENCIL_TEST);
		glStencilMask(0xFF);

		// Fail stencil test when inside of our newly rendered
		// inner portal
		glStencilFunc(GL_EQUAL, recursionLevel + 1, 0xFF);

		// Decrement stencil value on stencil fail
		// This resets the incremented values to what they were before,
		// eventually ending up with a stencil buffer full of zero's again
		// after the last (outer) step.
		glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

		// Draw portal into stencil buffer
		portal->draw(viewMat, projMat, portalShader);
	}

	// Disable the stencil test and stencil writing
	glDisable(GL_STENCIL_TEST);
	glStencilMask(0x00);

	// Disable color writing
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// Enable the depth test, and depth writing.
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	// Make sure we always write the data into the buffer
	glDepthFunc(GL_ALWAYS);

	// Clear the depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	// Draw portals into depth buffer
	for (auto& portal : portals)
		portal->draw(viewMat, projMat, portalShader);

	// Reset the depth function to the default
	glDepthFunc(GL_LESS);

	// Enable stencil test and disable writing to stencil buffer
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0x00);

	// Draw at stencil >= recursionlevel
	// which is at the current level or higher (more to the inside)
	// This basically prevents drawing on the outside of this level.
	glStencilFunc(GL_LEQUAL, recursionLevel, 0xFF);

	// Enable color and depth drawing again
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	// And enable the depth test
	glEnable(GL_DEPTH_TEST);

	// Draw scene objects normally, only at recursionLevel
	drawOtherObjects(viewMat, projMat, geoShader);
}

void drawLights(ew::Shader& _shader)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, fbWidth, fbHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	_shader.use();

	// Uniforms
	_shader.setMat4("_CameraViewproj", camera.projectionMatrix() * camera.viewMatrix());

	// Draw lights
	int lightsSize = sizeof(lights) / sizeof(lights[0]);
	for (int i = 0; i < lightsNum; i++)
	{
		_shader.setMat4("_TransformModel", glm::translate(lights[i].lightPos));
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
		_shader.setVec3("_Lights[" + std::to_string(i) + "].lightPos", lights[i].lightPos);
		_shader.setVec3("_Lights[" + std::to_string(i) + "].lightColor", lights[i].lightColor);
		_shader.setFloat("_Lights[" + std::to_string(i) + "].radius", lights[i].radius);
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

void drawGeometry(ew::Shader& _geoshader, ew::Model& _model, ew::Mesh& _plane, ew::Transform& _modelTransform, GLuint& _texture, ew::Shader portalShader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	// bind framebuffer here/
	// 1. pipeline definition
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	// 2. gfx pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClearStencil(0);

	//portal drawing
	recursiveDraw(camera.viewMatrix(), camera.projectionMatrix(), 2, 0, portalShader, _geoshader);

	glDisable(GL_STENCIL_TEST);

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
	ew::Shader portal_shader = ew::Shader("assets/portal.vert", "assets/portal.frag");

	suzanne = new ew::Model("assets/suzanne.obj");
	
	initPortals();

	brickTexture = ew::loadTexture("assets/brick_color.jpg");
	
	fullscreenQuad.initFullscreenQuad();
	framebuffer.Initialize();
	initLights();

	light.lightPos = glm::vec3(0.0, 2.0, 0.0);
	light.lightColor = glm::vec3(1.0, 0.0, 1.0);

	initModels();

	//Set up camera
	initCamera();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		portal1->transform.position = p1Position;
		portal1->transform.rotation = p1Rotation;
		portal2->transform.position = p2Position;
		portal2->transform.rotation = p2Rotation;

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//MOVE CAMERA
		camerController.move(window, &camera, deltaTime);

		//RENDER
		drawGeometry(geo_shader, *suzanne, plane, monkeyTransform, brickTexture, portal_shader);
		drawLighting(lit_shader);
		drawLights(lights_shader);

		drawUI();

		glfwSwapBuffers(window);
	}

	Cleanup();
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
	if (ImGui::Button("Reset Portals"))
	{
		p1Position = glm::vec3(0, 1.0f, 0);
		p2Position = glm::vec3(-5.0f, 1.0f, 0);
		p1Rotation = glm::vec3(0, 0, 0);
		p2Rotation = glm::vec3(0, 0, 0);
	}
	if (ImGui::Button("Reset Suzanne"))
	{
		monkeyTransform.position = glm::vec3(0, 0, 3);
		monkeyTransform.rotation = glm::vec3(0, 0, 0);
	}
	
	
	/*if (ImGui::CollapsingHeader("Material"))
	{
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 256.0f);
	}*/

	if (ImGui::CollapsingHeader("Portal 1"))
	{
		ImGui::SliderFloat3("Position", &p1Position.x, -7.0f, 7.0f);
		ImGui::SliderFloat3("Rotation", &p1Rotation.x, -7.0f, 7.0f);
	}
	if (ImGui::CollapsingHeader("Portal 2"))
	{
		ImGui::SliderFloat3("Position", &p2Position.x, -7.0f, 7.0f);
		ImGui::SliderFloat3("Rotation", &p2Rotation.x, -7.0f, 7.0f);
	}
	if (ImGui::CollapsingHeader("Suzanne"))
	{
		ImGui::SliderFloat3("Position", &monkeyTransform.position.x, -7.0f, 7.0f);
		ImGui::SliderFloat3("Rotation", &monkeyTransform.rotation.x, -7.0f, 7.0f);
	}
	//ImGui::SliderInt("Num Suzes", &suzaneNum, 1, 100);
	//ImGui::SliderFloat("Suzanne Spacer", &suzanneSpacer, 0.0f, 10.0f);
	//int totalSuzes = pow(2 * suzaneNum + 1, 2);
	//ImGui::Text("Number of suzes: ");
	//ImGui::Text(std::to_string(totalSuzes).c_str());

	if (ImGui::CollapsingHeader("Framebuffers"))
	{
		ImGui::Image((ImTextureID)(intptr_t)framebuffer.depth, ImVec2(fbWidth / 3, fbHeight / 3));
		ImGui::Image((ImTextureID)(intptr_t)framebuffer.color1, ImVec2(fbWidth, fbHeight));
		ImGui::Image((ImTextureID)(intptr_t)framebuffer.color2, ImVec2(fbWidth, fbHeight));
	}

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

