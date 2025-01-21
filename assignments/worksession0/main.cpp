#include <stdio.h>
#include <math.h>
#include <tuple>
#include <vector>
#include <string>

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

typedef struct
{
	glm::vec3 highlight;
	glm::vec3 shadow;
} Palette;

static int palette_index = 0;
static std::vector<std::tuple<std::string, Palette>> palette{
	{"Sunny Day", {{1.00f, 1.00f, 1.00f}, {0.60f, 0.54f, 0.52f}}},
	{"Bright Night", {{0.47f, 0.58f, 0.68f}, {0.32f, 0.39f, 0.57f}}},
	{"Rainy Day", {{0.62f, 0.69f, 0.67f}, {0.50f, 0.55f, 0.50f}}},
	{"Rainy Night", {{0.24f, 0.36f, 0.54f}, {0.25f, 0.31f, 0.31f}}},
};

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

//Cached data

ew::Camera camera;
ew::CameraController camerController;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

void initCamera()
{
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
}

void drawThing(ew::Shader& _shader, ew::Model& _model, ew::Transform& _modelTranform, GLuint& _texture1, GLuint& _texture2)
{
	// 1. pipeline definition
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);

	// 2. gfx pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _texture2);

	_modelTranform.rotation = glm::rotate(_modelTranform.rotation, deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));

	_shader.use();
	_shader.setInt("_Albedo", 0);
	_shader.setInt("_ZAtoon", 1);
	_shader.setMat4("_TransformModel", _modelTranform.modelMatrix());
	_shader.setMat4("_CameraViewproj", camera.projectionMatrix() * camera.viewMatrix());
	//Material uniforms
	_shader.setFloat("_Material.Ka", material.Ka);
	_shader.setFloat("_Material.Kd", material.Kd);
	_shader.setFloat("_Material.Ks", material.Ks);
	_shader.setFloat("_Material.Shininess", material.Shininess);

	//Palette
	_shader.setVec3("_Palette.highlight", std::get<Palette>(palette[palette_index]).highlight);
	_shader.setVec3("_Palette.shadow", std::get<Palette>(palette[palette_index]).shadow);

	_model.draw();
}

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//Set up assets
	ew::Shader toonShader = ew::Shader("assets/toon.vert", "assets/toon.frag");

	ew::Model skull = ew::Model("assets/skull.obj");
	ew::Transform monkeyTransform;
	monkeyTransform.scale = glm::vec3(0.1f);

	GLuint albedo = ew::loadTexture("assets/Txo_dokuo.png");
	GLuint toonTexture = ew::loadTexture("assets/ZAtoon.png");

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
		drawThing(toonShader, skull, monkeyTransform, albedo, toonTexture);

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

	if (ImGui::BeginCombo("Palette", std::get<std::string>(palette[palette_index]).c_str()))
	{
		for (auto n = 0; n < palette.size(); ++n)
		{
			auto is_selected = (std::get<0>(palette[palette_index]) == std::get<0>(palette[n]));
			if (ImGui::Selectable(std::get<std::string>(palette[n]).c_str(), is_selected))
			{
				palette_index = n;
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::ColorEdit3("Highlight", &std::get<Palette>(palette[palette_index]).highlight[0]);
	ImGui::ColorEdit3("Shadow", &std::get<Palette>(palette[palette_index]).shadow[0]);


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

