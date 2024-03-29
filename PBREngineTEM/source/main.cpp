#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <STB_IMAGE/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shader.h>
#include <camera.h>
#include <model.h>
#include <RenderEngine.h>

#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);

// settings
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
float xoffset = 0.0;
float yoffset = 0.0;
bool firstMouse = true;

// RenderEngine
RenderEngine* m_RenderEngine;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

												
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PBREngineTEM", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	m_RenderEngine = new RenderEngine(&camera, SCR_WIDTH, SCR_HEIGHT);

	
	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		//Update (Time, TimeStep, MousePos
		
		m_RenderEngine->Update(currentFrame, deltaTime, glm::vec2(xoffset, yoffset));

		// Render
		m_RenderEngine->Render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	xoffset = xpos - lastX;
	yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
	{
		m_RenderEngine->increaseDisplacement(0.1);
	}
	if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
	{
		m_RenderEngine->increaseDisplacement(-0.1);
	}

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		m_RenderEngine->increaseBrightness(0.1);
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		m_RenderEngine->increaseBrightness(-0.1);
	}

	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
	{
		m_RenderEngine->increaseContrast(0.1);
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
	{
		m_RenderEngine->increaseContrast(-0.1);

	}

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		m_RenderEngine->increaseSaturation(0.1);
	}
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		m_RenderEngine->increaseSaturation(-0.1);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		m_RenderEngine->switchShader();
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		m_RenderEngine->resetWeights();
	}
}