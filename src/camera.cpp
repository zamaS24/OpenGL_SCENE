#include"camera.h"

Camera::Camera(int width, int height, glm::vec3 position, glm::vec3 orientation, glm::vec3 fov_plane)
{
	Camera::width = width;
	Camera::height = height;
	Position = position;
	initPosition = position;
	Orientation = orientation;
	initOrientation = orientation;
	fovPlane = fov_plane;
}

void Camera::updateMatrix()
{
	view = glm::lookAt(Position, Position + Orientation, Up);
	projection = glm::perspective(glm::radians(fovPlane.x), (float)width / height, fovPlane.y, fovPlane.z);
	glUniform3fv(viewPos, 1, glm::value_ptr(Position));
	glUniformMatrix4fv(VPUniform, 1, GL_FALSE, glm::value_ptr(projection * view));
}


void Camera::setCameraUniform(Shader shaderProgram) {
	viewPos = glGetUniformLocation(shaderProgram.ID, "viewPos");
	VPUniform = glGetUniformLocation(shaderProgram.ID, "VP");
}

void Camera::keyboardInputs(GLFWwindow* window, int key, int action)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS )
	{
		Position += speed * Orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Position += speed * -glm::normalize(glm::cross(Orientation, Up));
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		Position += speed * -Orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Position += speed * glm::normalize(glm::cross(Orientation, Up));
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		Position = initPosition;
		Orientation = initOrientation;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		Position += speed * Up;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		Position += speed * -Up;
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		speed = speedHigh;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		speed = speedLow;
	}
}

void Camera::mouseInputs(GLFWwindow* window, double xpos, double ypos) {

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick)
		{
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		double mouseX;
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		//Normalisation de la position de la souris
		float rotX = sensitivity * (float)(mouseY - (height / static_cast<double>(2))) / height;
		float rotY = sensitivity * (float)(mouseX - (width / static_cast<double>(2))) / width;

		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

		// Pour que la camera ne se retourne pas
		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
		{
			Orientation = newOrientation;
		}

		// Rotates Gauche et droite
		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

		// Centrer la souris
		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}

}
