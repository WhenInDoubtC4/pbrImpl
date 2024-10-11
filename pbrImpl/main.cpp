/*
* Modified by Adam Gyenes
* Fork of assignment7
*/

#include <stdio.h>
#include <math.h>

#ifdef EMSCRIPTEN
#include <GLFW/emscripten_glfw3.h>
#include <GLES3/gl3.h>
#include <emscripten/html5.h>
#include <emscripten.h>
#else
#include <ew/external/glad.h>
#endif

#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <ew/shader.h>
#include <ew/texture.h>
#include <ew/procGen.h>
#include <ew/transform.h>
#include <ew/camera.h>
#include <ew/cameraController.h>

#include "util/Mesh.h"
#include "util/Texture.h"

#define _USE_MATH_DEFINES

#include <math.h>
#include <string>
#include <sstream>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void resetCamera(ew::Camera& camera, ew::CameraController& cameraController);

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

constexpr int MAX_LIGHTS = 4;

struct Light
{
	ew::Vec3 positon;
	ew::Vec3 color = ew::Vec3(1.f);
};

void renderLight(const Light& light, ew::Shader& emissiveShader, const ew::Mesh& lightMesh)
{
	ew::Transform lightTransform;
	lightTransform.position = light.positon;

	emissiveShader.setMat4("_Model", lightTransform.getModelMatrix());
	emissiveShader.setVec3("_lightColor", light.color);
	lightMesh.draw();
}

GLFWwindow* window;

float prevTime;
ew::Vec3 bgColor = ew::Vec3(0.1f);

ew::Camera camera;
ew::CameraController cameraController;

ew::Shader* shader;
GLuint colorTexture;
GLuint heightTexture;

ew::Shader* emissiveShader;

Util::Mesh* cubeMesh;
Util::Mesh* planeMesh;
Util::Mesh* sphereMesh;
Util::Mesh* cylinderMesh;

ew::Transform cubeTransform;
ew::Transform planeTransform;
ew::Transform sphereTransform;
ew::Transform cylinderTransform;

ew::Mesh* lightMesh;

bool animateLights = true;
int activeLights = MAX_LIGHTS;
float lightOrbitRadius = 3.f;
float lightOrbitSpeed = 1.f;
float lightHeight = 3.f;
Light lights[MAX_LIGHTS] =
{
	Light{ew::Vec3(0.f, lightHeight, lightOrbitRadius), ew::Vec3(1.f, 0.f, 0.f)},
	Light{ew::Vec3(lightOrbitRadius, lightHeight, 0.f), ew::Vec3(0.f, 1.f, 0.f)},
	Light{ew::Vec3(0.f, lightHeight, -lightOrbitRadius), ew::Vec3(0.f, 0.f, 1.f)},
	Light{ew::Vec3(-lightOrbitRadius, lightHeight, 0.f), ew::Vec3(1.f, 1.f, 0)}
};

float ambientK = 0.2f;
ew::Vec3 ambientColor = ew::Vec3(0.341f, 0.365f, 0.51f);
float diffuseK = 0.4f;
float specularK = 0.5f;
float shininess = 10.f;

//Parallax mapping settings
int parallaxMethod = 0;
bool discardOutOfBoundFrags = true;
float heightScale = 0.1f;
int minLayers = 8;
int maxLayers = 32;
int prevTextureUsed = 0;
int textureUsed = 0;

void cleanup()
{
	delete shader;
	delete emissiveShader;

	delete cubeMesh;
	delete planeMesh;
	delete sphereMesh;
	delete cylinderMesh;

	delete lightMesh;

	printf("Shutting down...");
}

void loop()
{
	glfwPollEvents();

	float time = (float)glfwGetTime();
	float deltaTime = time - prevTime;
	prevTime = time;

	//Update camera
	camera.aspectRatio = (float)SCREEN_WIDTH / SCREEN_HEIGHT;
	cameraController.Move(window, &camera, deltaTime);

	//Animate lights
	if (animateLights)
	{
		float lightAngleDiff = 2.f * M_PI / activeLights;
		for (int i = 0; i < activeLights; i++)
		{
			lights[i].positon = ew::Vec3(lightOrbitRadius * cos(time * lightOrbitSpeed + i * lightAngleDiff), lightHeight, lightOrbitRadius * sin(time * lightOrbitSpeed + i * lightAngleDiff));
		}
	}

	//RENDER
	glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightTexture);

	shader->use();
	shader->setInt("_colorTexture", 0);
	shader->setInt("_heightTexture", 1);
	shader->setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());
	shader->setVec3("_cameraPosition", camera.position);

	//Change light uniforms
	shader->setInt("_activeLights", activeLights);
	for (int i = 0; i < activeLights; i++)
	{
		std::stringstream lightPositionStream;
		lightPositionStream << "_lights[" << i << "].position";
		std::stringstream lightColorStream;
		lightColorStream << "_lights[" << i << "].color";

		shader->setVec3(lightPositionStream.str(), lights[i].positon);
		shader->setVec3(lightColorStream.str(), lights[i].color);
	}

	//Draw shapes
	shader->setMat4("_Model", cubeTransform.getModelMatrix());
	cubeMesh->draw();

	shader->setMat4("_Model", planeTransform.getModelMatrix());
	planeMesh->draw();

	shader->setMat4("_Model", sphereTransform.getModelMatrix());
	sphereMesh->draw();

	shader->setMat4("_Model", cylinderTransform.getModelMatrix());
	cylinderMesh->draw();

	//Set material/light props
	shader->setFloat("_material.ambientK", ambientK);
	shader->setVec3("_ambientColor", ambientColor);
	shader->setFloat("_material.diffuseK", diffuseK);
	shader->setFloat("_material.specularK", specularK);
	shader->setFloat("_material.shininess", shininess);

	//Set parallax mapping props
	shader->setInt("_parallaxMethod", parallaxMethod);
	shader->setInt("_discardOutOfBoundFrags", int(discardOutOfBoundFrags));
	shader->setFloat("_heightScale", heightScale);
	shader->setFloat("_minLayers", float(minLayers));
	shader->setFloat("_maxLayers", float(maxLayers));

	//Render point lights
	//Setup emissive shader
	emissiveShader->use();
	emissiveShader->setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());
	//Render all lights
	for (int i = 0; i < activeLights; i++)
	{
		renderLight(lights[i], *emissiveShader, *lightMesh);
	}

	//Render UI
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings");
		if (ImGui::CollapsingHeader("Camera")) {
			ImGui::DragFloat3("Position", &camera.position.x, 0.1f);
			ImGui::DragFloat3("Target", &camera.target.x, 0.1f);
			ImGui::Checkbox("Orthographic", &camera.orthographic);
			if (camera.orthographic) {
				ImGui::DragFloat("Ortho Height", &camera.orthoHeight, 0.1f);
			}
			else {
				ImGui::SliderFloat("FOV", &camera.fov, 0.0f, 180.0f);
			}
			ImGui::DragFloat("Near Plane", &camera.nearPlane, 0.1f, 0.0f);
			ImGui::DragFloat("Far Plane", &camera.farPlane, 0.1f, 0.0f);
			ImGui::DragFloat("Move Speed", &cameraController.moveSpeed, 0.1f);
			ImGui::DragFloat("Sprint Speed", &cameraController.sprintMoveSpeed, 0.1f);
			if (ImGui::Button("Reset")) {
				resetCamera(camera, cameraController);
			}
		}
		if (ImGui::CollapsingHeader("Material"))
		{
			ImGui::SliderFloat("Ambient light intensity", &ambientK, 0.f, 1.f);
			ImGui::ColorEdit3("Ambient light color", &ambientColor.x, ImGuiColorEditFlags_Float);
			ImGui::SliderFloat("Diffuse intensity", &diffuseK, 0.f, 1.f);
			ImGui::SliderFloat("Specular intenisty", &specularK, 0.f, 1.f);
			ImGui::DragFloat("Shininess", &shininess, 0.05f, 0.f);
		}
		if (ImGui::CollapsingHeader("Lights"))
		{
			ImGui::Indent();

			ImGui::SliderInt("Active lights", &activeLights, 1, MAX_LIGHTS);

			ImGui::Checkbox("Animate lights", &animateLights);
			ImGui::DragFloat("Light orbit radius", &lightOrbitRadius, 0.05f, 0.05f);
			ImGui::DragFloat("Light orbit speed", &lightOrbitSpeed, 0.05f, 0.05f);
			ImGui::DragFloat("Light height", &lightHeight, 0.05f);

			for (int i = 0; i < activeLights; i++)
			{
				ImGui::PushID(&lights[i]);
				if (ImGui::CollapsingHeader("Light"))
				{
					ImGui::DragFloat3("Position", &lights[i].positon.x, 0.05f);
					ImGui::ColorEdit3("Color", &lights[i].color.x, ImGuiColorEditFlags_Float);
				}
				ImGui::PopID();
			}

			ImGui::Unindent();
		}
		if (ImGui::CollapsingHeader("Parallax mapping"))
		{
			const char* parallaxMethodItems[] = { "Off", "Simple", "Steep", "Occlusion" };
			ImGui::Combo("Method", &parallaxMethod, parallaxMethodItems, 4);
			ImGui::Checkbox("Discard out of bound frags", &discardOutOfBoundFrags);
			ImGui::DragFloat("Height scale", &heightScale, 0.01f, 0.f);
			ImGui::DragInt("Min layers", &minLayers, 1.f, 1, 9999);
			ImGui::DragInt("Max layers", &maxLayers, 1.f, 2, 9999);
			const char* textureItems[] = { "Rock", "Bamboo" };
			ImGui::Combo("Texture", &textureUsed, textureItems, 2);
			if (prevTextureUsed != textureUsed)
			{
				if (textureUsed == 0)
				{
					colorTexture = Util::loadTexture("assets/rock_color.jpg", GL_REPEAT, GL_LINEAR);
					heightTexture = Util::loadTexture("assets/rock_height.jpg", GL_REPEAT, GL_LINEAR);
				}
				else
				{
					colorTexture = Util::loadTexture("assets/bamboo_color.jpg", GL_REPEAT, GL_LINEAR);
					heightTexture = Util::loadTexture("assets/bamboo_height.jpg", GL_REPEAT, GL_LINEAR);
				}

				prevTextureUsed = textureUsed;
			}
		}

		ImGui::ColorEdit3("BG color", &bgColor.x);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

#ifdef EMSCRIPTEN
	if (glfwWindowShouldClose(window))
	{
		emscripten_cancel_main_loop();
		cleanup();
	}
#endif
}

int main() {
	printf("Initializing...");

	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return 1;
	}

	double devicePixelRatio = 1;

#ifdef EMSCRIPTEN
	// setting the association window <-> canvas
	emscripten_glfw_set_next_window_canvas_selector("#canvas");
	devicePixelRatio = emscripten_get_device_pixel_ratio();
#endif

	//Truly beautiful
	window = glfwCreateWindow(SCREEN_WIDTH * devicePixelRatio, SCREEN_HEIGHT * devicePixelRatio, "TEMPLATE", NULL, NULL);

	if (window == NULL) {
		printf("GLFW failed to create window");
		return 1;
	}

#ifdef EMSCRIPTEN
	//makes the canvas resizable and match the container
	emscripten_glfw_make_canvas_resizable(window, "#canvas-container", nullptr);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwSetWindowAttrib(window, GLFW_SCALE_FRAMEBUFFER, true);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

#ifndef EMSCRIPTEN
	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return 1;
	}
#endif

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Global settings
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	colorTexture = Util::loadTexture("assets/rock_color.jpg", GL_REPEAT, GL_LINEAR);
	heightTexture = Util::loadTexture("assets/rock_height.jpg", GL_REPEAT, GL_LINEAR);

	shader = new ew::Shader("assets/defaultLit.vert", "assets/defaultLit.frag");
	emissiveShader = new ew::Shader("assets/emissive.vert", "assets/emissive.frag");

	cubeMesh = new Util::Mesh(ew::createCube(1.0f));
	planeMesh = new Util::Mesh(ew::createPlane(5.0f, 5.0f, 10));
	sphereMesh = new Util::Mesh(ew::createSphere(0.5f, 64));
	cylinderMesh = new Util::Mesh(ew::createCylinder(0.5f, 1.0f, 32));

	planeTransform.position = ew::Vec3(0, -1.0, 0);
	sphereTransform.position = ew::Vec3(-1.5f, 0.0f, 0.0f);
	cylinderTransform.position = ew::Vec3(1.5f, 0.0f, 0.0f);

	lightMesh = new ew::Mesh(ew::createSphere(0.3f, 12));

	resetCamera(camera,cameraController);

	int wWidth, wHeight;
	glfwGetWindowSize(window, &wWidth, &wHeight);
	framebufferSizeCallback(window, wWidth, wHeight);

#ifdef EMSCRIPTEN
	emscripten_set_main_loop(loop, 0, GLFW_FALSE);
#else
	while (!glfwWindowShouldClose(window)) 
	{
		loop();

		glfwSwapBuffers(window);
	}

	cleanup();
#endif
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	double devicePixelRatio = 1;
#ifdef EMSCRIPTEN
	devicePixelRatio = emscripten_get_device_pixel_ratio();
#endif

	glViewport(0, 0, width * devicePixelRatio, height * devicePixelRatio);
	SCREEN_WIDTH = width * devicePixelRatio;
	SCREEN_HEIGHT = height * devicePixelRatio;

	printf("Resize to %ix%i *%f\n", width, height, devicePixelRatio);
}

void resetCamera(ew::Camera& camera, ew::CameraController& cameraController) {
	camera.position = ew::Vec3(0, 0, 5);
	camera.target = ew::Vec3(0);
	camera.fov = 60.0f;
	camera.orthoHeight = 6.0f;
	camera.nearPlane = 0.1f;
	camera.farPlane = 100.0f;
	camera.orthographic = false;

	cameraController.yaw = 0.0f;
	cameraController.pitch = 0.0f;
}


