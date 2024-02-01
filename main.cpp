#include "camera.h"
#include "shader.h"
#include "assimp_model.h"
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <assimp/types.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const int width = 800;
const int height = 800;


static void Reshape(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

// Position de la camera
glm::vec3 position = glm::vec3(0.0f, 25.0f, -20.0f);
// L'orientation de la camera (ou elle regarde)
glm::vec3 orientation = glm::vec3(0.0f, -0.5f, 1.0f);
// field of view, near plane, far plane
glm::vec3 fov_plane = glm::vec3(45.0f, 0.1f, 200.0f);

Camera camera(width, height, position, orientation, fov_plane);

int main() {

	if (!glfwInit()) {
		std::cout << "Could not initialize GLFW" << std::endl;
		exit(1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Final Project", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(1);
	}
	glfwMakeContextCurrent(window);

	gladLoadGL();

	glfwSetFramebufferSizeCallback(window, Reshape);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	Shader shaderProgram("shaders/shader.vert", "shaders/shader.frag");
	shaderProgram.Activate();
	camera.setCameraUniform(shaderProgram);


	//BasicMesh model;
	//model.LoadMesh("./models/sphere.obj", 1);
	//model.WorldMats[0] = glm::translate(glm::vec3(0, 0, 0));
	
	BasicMesh terrain;
	terrain.LoadMesh("./models/field_and_garden/scene.gltf", 1);


	BasicMesh maison;
	maison.LoadMesh("./models/farm_house/scene.gltf", 1);
	maison.WorldMats[0] = glm::scale(maison.WorldMats[0], glm::vec3(0.1, 0.1, 0.1));
	maison.WorldMats[0] = glm::translate(maison.WorldMats[0], glm::vec3(-40, 14, 300));
	maison.WorldMats[0] = glm::rotate(maison.WorldMats[0], glm::radians(-90.0f), glm::vec3(0,1,0));

	BasicMesh vase;
	vase.LoadMesh("./models/antique_ceramic_vase_01_4k.blend/antique_ceramic_vase_01_4k.obj", 2);
	vase.WorldMats[0] = glm::translate(vase.WorldMats[0], glm::vec3(-4.2f, -3.5f, 23.5f));
	vase.WorldMats[0] = glm::scale(vase.WorldMats[0], glm::vec3(3, 3, 3));
	vase.WorldMats[1] = glm::translate(vase.WorldMats[1], glm::vec3(-5.8f, -3.5f, 24.5f));
	vase.WorldMats[1] = glm::scale(vase.WorldMats[1], glm::vec3(3,3,3));

	BasicMesh coop;
	coop.LoadMesh("./models/chicken_coop_free/scene.gltf", 1);
	coop.WorldMats[0] = glm::translate(coop.WorldMats[0], glm::vec3(10, -2.3f, 20));
	coop.WorldMats[0] = glm::rotate(coop.WorldMats[0], glm::radians(-90.0f), glm::vec3(0, 1, 0));
	coop.WorldMats[0] = glm::scale(coop.WorldMats[0], glm::vec3(0.01, 0.01, 0.01));

	BasicMesh chicken;
	chicken.LoadMesh("./models/chicken/scene.gltf", 10);
	float x = 7, y = -3, z = 20;
	for (int i = 0; i < 10; i++) {
		chicken.WorldMats[i] = glm::translate(chicken.WorldMats[i], glm::vec3(x, y, z));
		chicken.WorldMats[i] = glm::rotate(chicken.WorldMats[i], glm::radians((float)rand()), glm::vec3(0, 1, 0));
		//chicken.WorldMats[i] = glm::scale(chicken.WorldMats[i], glm::vec3(10, 10, 10));
		z -= 2;
		x -= 0.5;
	}

	BasicMesh cow;
	cow.LoadMesh("./models/cow/scene.gltf", 3);
	x = 0 , y = -3.5f, z = 15;
	for (int i = 0; i < 3; i++) {
		cow.WorldMats[i] = glm::translate(cow.WorldMats[i], glm::vec3(x, y, z));
		cow.WorldMats[i] = glm::scale(cow.WorldMats[i], glm::vec3(0.1, 0.1, 0.1));
		cow.WorldMats[i] = glm::rotate(cow.WorldMats[i], glm::radians((float)rand()), glm::vec3(0, 1, 0));
		z += 2;
		x -= 2;
	}

	// Diffuse light
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(100.0f, 100.0f, -100.0f);
	glUniform3fv(shaderProgram.lightColor, 1, glm::value_ptr(lightColor));
	glUniform3fv(shaderProgram.lightPositon, 1, glm::value_ptr(lightPos));
	glUniform1f(shaderProgram.lightDiffuseIntensity, 0.8f);
	//Ambient light
	glUniform1f(shaderProgram.lightAmbientIntensity, 0.5f);
	

	glEnable(GL_DEPTH_TEST);


	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	double prevTime = 0.0;
	double crntTime = 0.0;
	double timeDiff;
	unsigned int counter = 0;

	while (!glfwWindowShouldClose(window))
	{
		crntTime = glfwGetTime();
		timeDiff = crntTime - prevTime;
		counter++;
		// Boucle pour afficher le FPS et le temps de rendu
		if (timeDiff >= 1.0 / 30.0)
		{
			std::string FPS = std::to_string((1.0 / timeDiff) * counter);
			std::string ms = std::to_string((timeDiff / counter) * 1000);
			std::string newTitle = "Final Project - " + FPS + "FPS / " + ms + "ms";
			glfwSetWindowTitle(window, newTitle.c_str());
			prevTime = crntTime;
			counter = 0;
		}
		//Limiter le FPS a 60fps
		double lasttime = glfwGetTime();
		while (glfwGetTime() < lasttime + 1.0 / 60) {}
		lasttime += 1.0 / 60;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//model.Render(shaderProgram);
		terrain.Render(shaderProgram);
		maison.Render(shaderProgram);
		vase.Render(shaderProgram);
		coop.Render(shaderProgram);
		chicken.Render(shaderProgram);
		cow.Render(shaderProgram);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//Fin de program et nettoyage
	shaderProgram.Delete();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

static void Reshape(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	camera.keyboardInputs(window, key, action);
	camera.updateMatrix();
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	camera.mouseInputs(window, xpos, ypos);
	camera.updateMatrix();
}

