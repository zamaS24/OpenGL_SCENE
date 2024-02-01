#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include "shader.h"

class Camera
{
public:
	// Stores the main vectors of the camera
	glm::vec3 Position;
	glm::vec3 initPosition;
	glm::vec3 Orientation;
	glm::vec3 initOrientation;
	glm::vec3 fovPlane;
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	
	bool firstClick = true;

	
	int width;
	int height;

	
	float speedHigh = 2.0f;
	float speedLow = 0.1f;
	float speed = speedLow;
	
	float sensitivity = 100.0f;
	GLuint VPUniform = 0;
	GLuint viewPos;
	
	Camera(int width, int height, glm::vec3 position, glm::vec3 orientation, glm::vec3 fov_plane);

	void updateMatrix();
	void setCameraUniform(Shader shaderProgram);
	void keyboardInputs(GLFWwindow* window, int key, int action);
	void mouseInputs(GLFWwindow* window, double xpos, double ypos);
};
#endif