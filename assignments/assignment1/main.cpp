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

#include <js/framebuffer.h>

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

static int effect_index = 0;
static std::vector<std::string> post_processing_effects = {
	"None",
	"Grayscale",
	"Gaussian Blur",
	"Sharpen Blur",
	"Edge Blur",
	"Inverse",
	"Chromatic Aberration",
	"Vignette",
	"HDR",
	"Lens Distortion",
	"Film Grain",
	"Fog"
};

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

//js::Framebuffer framebufferHDR = js::createFramebuffer(screenWidth, screenHeight); weird null reference
struct FrameBufferHDR
{
	GLuint fbo;
	GLuint color0;
	GLuint color1;
	GLuint depth;

	void Initialize()
	{
		glGenFramebuffers(1, &framebufferHDR.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferHDR.fbo);

		// color attachment
		glGenTextures(1, &framebufferHDR.color0);
		glBindTexture(GL_TEXTURE_2D, framebufferHDR.color0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferHDR.color0, 0);

		// Other color attach for HDR
		glGenTextures(1, &framebufferHDR.color1);
		glBindTexture(GL_TEXTURE_2D, framebufferHDR.color1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); //Changed from GL_RGB to GL_RGB16F
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebufferHDR.color1, 0);

		// Depth attachment
		glGenTextures(1, &framebufferHDR.depth);
		glBindTexture(GL_TEXTURE_2D, framebufferHDR.depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, framebufferHDR.depth, 0);

		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		//check completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("Framebuffer incomplete: %d", 1);
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
} framebufferHDR;

// Uniform data
// Blur
float strength = 10;
// HDR
float exposure = 1.0f;
float gamma = 2.2f;
// Fisheye uniform
float distortion = 0.0f;
// Film grain uniform
float noiseModifier = 0.5f;
float time = 0.0f;
// Fog
float fogNear = 2.0f;
float fogFar = 10.0f;

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


void drawThing(ew::Shader& _shader, ew::Model& _model, ew::Transform& _modelTranform, GLuint& _texture)
{
	// Bind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferHDR.fbo);

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

	// Uniforms
	_shader.setFloat("_Exposure", exposure);
	_shader.setFloat("_Gamma", gamma);
	_shader.setFloat("_Distortion", distortion);
	_shader.setFloat("_NoiseModifier", noiseModifier);
	_shader.setFloat("_Time", time);
	_shader.setFloat("_Strength", strength);
	_shader.setFloat("_FogNear", fogNear);
	_shader.setFloat("_FogFar", fogFar);

	glBindVertexArray(fullscreenQuad.vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebufferHDR.color0);

	//Bind depth for fog
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, framebufferHDR.depth);
	_shader.setInt("_DepthTexture", 1);
	_shader.setVec3("_CameraPos", camera.position);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void drawFrameBufferLIT(ew::Shader& _shader, ew::Model& _model, ew::Transform& _modelTranform, GLuint& _texture)
{
	glDisable(GL_DEPTH_TEST);

	_shader.use();
	_shader.setInt("_MainTexture", 0);
	_shader.setMat4("_TransformModel", _modelTranform.modelMatrix());
	_shader.setMat4("_CameraViewproj", camera.projectionMatrix() * camera.viewMatrix());
	//Material uniforms
	_shader.setFloat("_Material.Ka", material.Ka);
	_shader.setFloat("_Material.Kd", material.Kd);
	_shader.setFloat("_Material.Ks", material.Ks);
	_shader.setFloat("_Material.Shininess", material.Shininess);


	glBindVertexArray(fullscreenQuad.vao);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebufferHDR.color1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture);
	_model.draw();

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//Set up assets
	ew::Shader lit_shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader full_shader = ew::Shader("assets/fullscreen.vert", "assets/fullscreen.frag");
	ew::Shader inverse_shader = ew::Shader("assets/inverse.vert", "assets/inverse.frag");
	ew::Shader grayscale_shader = ew::Shader("assets/grayscale.vert", "assets/grayscale.frag");
	ew::Shader gaussian_shader = ew::Shader("assets/gaussianBlur.vert", "assets/gaussianBlur.frag");
	ew::Shader sharpen_shader = ew::Shader("assets/sharpenBlur.vert", "assets/sharpenBlur.frag");
	ew::Shader edge_shader = ew::Shader("assets/edgeBlur.vert", "assets/edgeBlur.frag");
	ew::Shader chromatic_shader = ew::Shader("assets/chromatic.vert", "assets/chromatic.frag");
	ew::Shader vignette_shader = ew::Shader("assets/vignette.vert", "assets/vignette.frag");
	ew::Shader hdr_shader = ew::Shader("assets/hdr.vert", "assets/hdr.frag");
	ew::Shader fisheye_shader = ew::Shader("assets/fisheye.vert", "assets/fisheye.frag");
	ew::Shader filmgrain_shader = ew::Shader("assets/filmgrain.vert", "assets/filmgrain.frag");
	ew::Shader fog_shader = ew::Shader("assets/fog.vert", "assets/fog.frag");

	ew::Model suzanne = ew::Model("assets/suzanne.obj");
	ew::Transform monkeyTransform;

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	//Set up camera
	initCamera();

	initFullscreenQuad();

	framebufferHDR.Initialize();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		// Clear default buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);

		//MOVE CAMERA
		camerController.move(window, &camera, deltaTime);

		//RENDER
		drawThing(lit_shader, suzanne, monkeyTransform, brickTexture);

		switch (effect_index)
		{
		case 1:
			drawFrameBuffer(grayscale_shader);
			break;
		case 2:
			strength = 10.0f;
			drawFrameBuffer(gaussian_shader);
			break;
		case 3:
			strength = 10.0f;
			drawFrameBuffer(sharpen_shader);
			break;
		case 4:
			strength = 0.3f;
			drawFrameBuffer(edge_shader);
			break;
		case 5:
			drawFrameBuffer(inverse_shader);
			break;
		case 6:
			drawFrameBuffer(chromatic_shader);
			break;
		case 7:
			drawFrameBuffer(vignette_shader);
			break;
		case 8:
			drawFrameBuffer(hdr_shader);
			break;
		case 9:
			drawFrameBuffer(fisheye_shader);
			break;
		case 10:
			drawFrameBuffer(filmgrain_shader);
			break;
		case 11:
			drawFrameBuffer(fog_shader);
			break;
		default:
			drawFrameBufferLIT(lit_shader, suzanne, monkeyTransform, brickTexture);
			break;
		}

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

	if (ImGui::BeginCombo("Effect", post_processing_effects[effect_index].c_str()))
	{
		for (auto n = 0; n < post_processing_effects.size(); ++n)
		{
			auto is_selected = (post_processing_effects[effect_index] == post_processing_effects[n]);
			if (ImGui::Selectable(post_processing_effects[n].c_str(), is_selected))
			{
				effect_index = n;
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	// Uniforms
	if (effect_index >= 2 && effect_index <= 4)
	{
		ImGui::SliderFloat("Strength", &strength, -20.0f, 20.0f);
	}
	else if (effect_index == 8)
	{
		ImGui::SliderFloat("Exposure", &exposure, 0.0f, 10.0f);
		ImGui::SliderFloat("Gamma", &gamma, 0.0f, 10.0f);
	}
	else if (effect_index == 9)
	{
		ImGui::SliderFloat("Distortion", &distortion, -10.0f, 10.0f);
	}
	else if (effect_index == 10)
	{
		ImGui::SliderFloat("Noise", &noiseModifier, 0.0f, 5.0f);
	}
	else if (effect_index == 11)
	{
		ImGui::SliderFloat("Near", &fogNear, 0.1f, 10.0f);
		ImGui::SliderFloat("Far", &fogFar, 5.0f, 20.0f);
	}

	/*ImGui::Image((ImTextureID)(intptr_t)framebufferHDR.color0, ImVec2(screenWidth, screenHeight));
	ImGui::Image((ImTextureID)(intptr_t)framebufferHDR.depth, ImVec2(screenWidth, screenHeight));*/

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

