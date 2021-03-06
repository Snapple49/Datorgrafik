// Assignment 3, Part 1 and 2
//
// Modify this file according to the lab instructions.
//

#include "utils.h"
#include "utils2.h"
#include "stb_image.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>
#include <algorithm>

glm::vec3 shader_switch = glm::vec3(1.0, 1.0, 1.0);
glm::vec4 stroke_color = glm::vec4(0.0, 0.0, 0.0, 1.0);
GLboolean bShowNormalsAsRgb = false;
GLboolean bUseGammaCorrection = true;
float zoomFactor = 1.0;
float toonShading_a = 0.7f;
float toonShading_b = 0.3f;
float crossHatch_variable = 0.0f;

// The attribute locations we will use in the vertex shader
enum AttributeLocation {
	POSITION = 0,
	NORMAL = 1
};

// Struct for representing an indexed triangle mesh
struct Mesh {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<uint32_t> indices;
};

// Struct for representing a vertex array object (VAO) created from a
// mesh. Used for rendering.
struct MeshVAO {
	GLuint vao;
	GLuint vertexVBO;
	GLuint normalVBO;
	GLuint indexVBO;
	int numVertices;
	int numIndices;
};

// Struct for resources and state
struct Context {
	int width;
	int height;
	float aspect;
	GLFWwindow *window;
	GLuint program;
	Trackball trackball;
	Mesh mesh;
	MeshVAO meshVAO;
	GLuint defaultVAO;
	GLuint cubemap;
	float elapsed_time;
	GLenum activeTexture;
	//texturestuff
	unsigned char* data[6];
	int tex_height[6];
	int tex_width[6];
	int nr_channels[6];
};

// Returns the value of an environment variable
std::string getEnvVar(const std::string &name)
{
	char const* value = std::getenv(name.c_str());
	if (value == nullptr) {
		return std::string();
	}
	else {
		return std::string(value);
	}
}

// Returns the absolute path to the shader directory
std::string shaderDir(void)
{
	std::string rootDir = getEnvVar("ASSIGNMENT3_ROOT");
	if (rootDir.empty()) {
		std::cout << "Error: ASSIGNMENT3_ROOT is not set." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return rootDir + "/model_viewer/src/shaders/";
}

// Returns the absolute path to the 3D model directory
std::string modelDir(void)
{
	std::string rootDir = getEnvVar("ASSIGNMENT3_ROOT");
	if (rootDir.empty()) {
		std::cout << "Error: ASSIGNMENT3_ROOT is not set." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return rootDir + "/model_viewer/3d_models/";
}

// Returns the absolute path to the cubemap texture directory
std::string cubemapDir(void)
{
	std::string rootDir = getEnvVar("ASSIGNMENT3_ROOT");
	if (rootDir.empty()) {
		std::cout << "Error: ASSIGNMENT3_ROOT is not set." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return rootDir + "/model_viewer/cubemaps/";
}

// Returns the absolute path to the texture directory
std::string textureDir(std::string name)
{
	std::string rootDir = getEnvVar("ASSIGNMENT3_ROOT");
	if (rootDir.empty()) {
		std::cout << "Error: ASSIGNMENT3_ROOT is not set." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return rootDir + "/model_viewer/textures/" + name;
}

void loadMesh(const std::string &filename, Mesh *mesh)
{
	OBJMesh obj_mesh;
	objMeshLoad(obj_mesh, filename);
	mesh->vertices = obj_mesh.vertices;
	mesh->normals = obj_mesh.normals;
	mesh->indices = obj_mesh.indices;
}

void createMeshVAO(Context &ctx, const Mesh &mesh, MeshVAO *meshVAO)
{
	// Generates and populates a VBO for the vertices
	glGenBuffers(1, &(meshVAO->vertexVBO));
	glBindBuffer(GL_ARRAY_BUFFER, meshVAO->vertexVBO);
	auto verticesNBytes = mesh.vertices.size() * sizeof(mesh.vertices[0]);
	glBufferData(GL_ARRAY_BUFFER, verticesNBytes, mesh.vertices.data(), GL_STATIC_DRAW);

	// Generates and populates a VBO for the vertex normals
	glGenBuffers(1, &(meshVAO->normalVBO));
	glBindBuffer(GL_ARRAY_BUFFER, meshVAO->normalVBO);
	auto normalsNBytes = mesh.normals.size() * sizeof(mesh.normals[0]);
	glBufferData(GL_ARRAY_BUFFER, normalsNBytes, mesh.normals.data(), GL_STATIC_DRAW);

	// Generates and populates a VBO for the element indices
	glGenBuffers(1, &(meshVAO->indexVBO));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshVAO->indexVBO);
	auto indicesNBytes = mesh.indices.size() * sizeof(mesh.indices[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesNBytes, mesh.indices.data(), GL_STATIC_DRAW);

	// Creates a vertex array object (VAO) for drawing the mesh
	glGenVertexArrays(1, &(meshVAO->vao));
	glBindVertexArray(meshVAO->vao);
	glBindBuffer(GL_ARRAY_BUFFER, meshVAO->vertexVBO);
	glEnableVertexAttribArray(POSITION);
	glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, meshVAO->normalVBO);
	glEnableVertexAttribArray(NORMAL);
	glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshVAO->indexVBO);
	glBindVertexArray(ctx.defaultVAO); // unbinds the VAO

									   // Additional information required by draw calls
	meshVAO->numVertices = mesh.vertices.size();
	meshVAO->numIndices = mesh.indices.size();
}

void initializeTrackball(Context &ctx)
{
	double radius = double(std::min(ctx.width, ctx.height)) / 2.0;
	ctx.trackball.radius = radius;
	glm::vec2 center = glm::vec2(ctx.width, ctx.height) / 2.0f;
	ctx.trackball.center = center;
}

GLuint textures[6];

void init(Context &ctx)
{
	ctx.program = loadShaderProgram(shaderDir() + "mesh.vert",
		shaderDir() + "mesh.frag");

	loadMesh((modelDir() + "bunny.obj"), &ctx.mesh);
	createMeshVAO(ctx, ctx.mesh, &ctx.meshVAO);

	// Load cubemap texture(s)
	// ...
	int width, height, nrChannels;
	for (size_t i = 0; i < 6; i++)
	{
		ctx.data[i] = stbi_load((textureDir("128_" + std::to_string(i) + ".png")).c_str(), &ctx.tex_width[i], &ctx.tex_height[i], &ctx.nr_channels[i], 0);
		if (ctx.data[i]) {
			glGenTextures(1, &textures[i]);
			glBindTexture(GL_TEXTURE_2D, textures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ctx.tex_width[i], ctx.tex_height[i], 0, GL_RGB, GL_UNSIGNED_BYTE, ctx.data[i]);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			std::cout << "Failed to load texture" << std::endl;
			// stbi_failure_reason()
		}
	}
	
	initializeTrackball(ctx);
}

// MODIFY THIS FUNCTION
void drawMesh(Context &ctx, GLuint program, const MeshVAO &meshVAO)
{
	// Define uniforms
	glm::mat4 model = trackballGetRotationMatrix(ctx.trackball);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 8), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); // glm::mat4();
	glm::mat4 projection = glm::perspective(glm::radians(30.0f)*zoomFactor, ctx.aspect, 0.1f, 150.0f); //glm::ortho(-ctx.aspect, ctx.aspect, -1.0f, 1.0f, -1.0f, 1.0f);
	glm::mat4 mv = view * model;
	glm::mat4 mvp = projection * mv;
	// ...

	// Load texture
	
	

	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	//glEnableVertexAttribArray(2);

	// Activate program
	glUseProgram(program);

	// Bind textures
	// ...
	for (int i = 0; i < 6; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glUniform1i(glGetUniformLocation(ctx.program, ("u_texture_" + std::to_string(i)).c_str()), i);
	}

	// light
	glm::vec3 light_position = glm::vec3(1.0f, 1.0f, -1.0f);
	glUniform3fv(glGetUniformLocation(ctx.program, "u_light_position"), 1, &light_position[0]);
	glm::vec3 light_color = glm::vec3(1.0f, 1.0f, 1.0f);
	glUniform3fv(glGetUniformLocation(ctx.program, "u_light_color"), 1, &light_color[0]);

	glUniform3fv(glGetUniformLocation(ctx.program, "u_shader_switch"), 1, &shader_switch[0]);
	glUniform4fv(glGetUniformLocation(ctx.program, "u_stroke_color"), 1, &stroke_color[0]);


	// Pass uniforms
	glUniformMatrix4fv(glGetUniformLocation(program, "u_v"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "u_mv"), 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "u_mvp"), 1, GL_FALSE, &mvp[0][0]);
	glUniform1f(glGetUniformLocation(program, "u_toonA"), toonShading_a);
	glUniform1f(glGetUniformLocation(program, "u_toonB"), toonShading_b);
	// ...

	// Draw!
	glBindVertexArray(meshVAO.vao);
	glDrawElements(GL_TRIANGLES, meshVAO.numIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(ctx.defaultVAO);
//	ImGui::ShowTestWindow();
}

void display(Context &ctx)
{
	glClearColor(0.9, 0.9, 0.9, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST); // ensures that polygons overlap correctly
	drawMesh(ctx, ctx.program, ctx.meshVAO);
}

void reloadShaders(Context *ctx)
{
	glDeleteProgram(ctx->program);
	ctx->program = loadShaderProgram(shaderDir() + "mesh.vert",
		shaderDir() + "mesh.frag");
}

void mouseButtonPressed(Context *ctx, int button, int x, int y)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		ctx->trackball.center = glm::vec2(x, y);
		trackballStartTracking(ctx->trackball, glm::vec2(x, y));
	}
}

void mouseButtonReleased(Context *ctx, int button, int x, int y)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		trackballStopTracking(ctx->trackball);
	}
}

void moveTrackball(Context *ctx, int x, int y)
{
	if (ctx->trackball.tracking) {
		trackballMove(ctx->trackball, glm::vec2(x, y));
	}
}

void errorCallback(int /*error*/, const char* description)
{
	std::cerr << description << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Forward event to GUI
	ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
	if (ImGui::GetIO().WantCaptureKeyboard) { return; }  // Skip other handling

	Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		reloadShaders(ctx);
	}
	else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		if (shader_switch.x == 0.0) {
			shader_switch.x = 1.0;
		}
		else shader_switch.x = 0.0;
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		if (shader_switch.y == 0.0) {
			shader_switch.y = 1.0;
		}
		else shader_switch.y = 0.0;
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		if (shader_switch.z == 0.0) {
			shader_switch.z = 1.0;
		}
		else shader_switch.z = 0.0;
	}
}

void charCallback(GLFWwindow* window, unsigned int codepoint)
{
	// Forward event to GUI
	ImGui_ImplGlfwGL3_CharCallback(window, codepoint);
	if (ImGui::GetIO().WantTextInput) { return; }  // Skip other handling
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	// Forward event to GUI
	ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);
	if (ImGui::GetIO().WantCaptureMouse) { return; }  // Skip other handling

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
	if (action == GLFW_PRESS) {
		mouseButtonPressed(ctx, button, x, y);
	}
	else {
		mouseButtonReleased(ctx, button, x, y);
	}
}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
	if (ImGui::GetIO().WantCaptureMouse) { return; }  // Skip other handling   

	Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
	moveTrackball(ctx, x, y);
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
	Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
	ctx->width = width;
	ctx->height = height;
	ctx->aspect = float(width) / float(height);
	ctx->trackball.radius = double(std::min(width, height)) / 2.0;
	ctx->trackball.center = glm::vec2(width, height) / 2.0f;
	glViewport(0, 0, width, height);
}

int main(void)
{
	Context ctx;

	// Create a GLFW window
	glfwSetErrorCallback(errorCallback);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	ctx.width = 500;
	ctx.height = 500;
	ctx.aspect = float(ctx.width) / float(ctx.height);
	ctx.window = glfwCreateWindow(ctx.width, ctx.height, "Model viewer", nullptr, nullptr);
	glfwMakeContextCurrent(ctx.window);
	glfwSetWindowUserPointer(ctx.window, &ctx);
	glfwSetKeyCallback(ctx.window, keyCallback);
	glfwSetCharCallback(ctx.window, charCallback);
	glfwSetMouseButtonCallback(ctx.window, mouseButtonCallback);
	glfwSetCursorPosCallback(ctx.window, cursorPosCallback);
	glfwSetFramebufferSizeCallback(ctx.window, resizeCallback);

	// Load OpenGL functions
	glewExperimental = true;
	GLenum status = glewInit();
	if (status != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(status) << std::endl;
		std::exit(EXIT_FAILURE);
	}
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	// Initialize GUI
	ImGui_ImplGlfwGL3_Init(ctx.window, false /*do not install callbacks*/);

	
	// Initialize rendering
	glGenVertexArrays(1, &ctx.defaultVAO);
	glBindVertexArray(ctx.defaultVAO);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	init(ctx);


	static bool show_app_main_menu_bar = true;
	


	// Start rendering loop
	while (!glfwWindowShouldClose(ctx.window)) {
	

		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();
		ctx.elapsed_time = glfwGetTime();
		
	
		ImGui::Spacing();
		ImGui::Separator();

		ImGui::Text("ToonAdjust_a");
		ImGui::SliderFloat("Wrap width2", &toonShading_a, 0.2, 3.0, "%.3f");
		ImGui::NextColumn();

		ImGui::Text("ToonAdjust_b");
		static float bar = 1.0f;
		ImGui::SliderFloat("Wrap width", &toonShading_b, -2.0, 1.0, "%.3f");
		ImGui::NextColumn();
					
		//zoom
		ImGui::Text("ZOOM");
		ImGui::SliderFloat("zoom", &zoomFactor, 0.1f, 2, "%.3f");
		ImGui::NextColumn();
		ImGui::Separator();


		//Light controls
		ImGui::Text("LIGHT");
		ImGui::SliderFloat("Diffuse", &shader_switch.x, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Ambient", &shader_switch.y, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Specular", &shader_switch.z, 0.0f, 1.0f, "%.2f");
		ImGui::NextColumn();
		ImGui::Separator();

		//Stroke color
		ImGui::Text("STROKE COLOR");
		ImGui::SliderFloat("Red", &stroke_color.r, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Green", &stroke_color.g, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Blue", &stroke_color.b, 0.0f, 1.0f, "%.2f");
		ImGui::NextColumn();
		ImGui::Separator();

		//ImGui_ImplGlfwGL3_NewFrame();
		display(ctx);
		ImGui::Render();
		glfwSwapBuffers(ctx.window);
	}

	// Shutdown
	glfwDestroyWindow(ctx.window);
	glfwTerminate();
	std::exit(EXIT_SUCCESS);
}