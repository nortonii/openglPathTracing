#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include "shader.h"
#include "camera.h"
#include "model.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <initializer_list>
#include <cstdlib> 
#include <ctime>
#include <stdlib.h>
#include "imageGet.h"
#include "tbotool.h"
#include "ray.h"
#include "createBVH.h"
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
// camera
Camera camera(glm::vec3(0.2764f, 0.2744f, -0.7f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("resource/vertex.vert", "resource/frame.frag");

	// load models
	// -----------
	//Model ourModel("resource/models/bunny/bunny.obj");
	Model ourModel1("resource/models/cornellbox/tallbox.obj"); 
	Model ourModel2("resource/models/cornellbox/floor.obj");
	Model ourModel3("resource/models/cornellbox/shortbox.obj");
	Model ourModel4("resource/models/cornellbox/left.obj");
	Model ourModel5("resource/models/cornellbox/right.obj");
	Model ourModel6("resource/models/cornellbox/light.obj");
	vector<triangle> triangleList;
	ourModel1.obj2BVH(triangleList, glm::vec3(0.725f, 0.71f, 0.68f));
	ourModel2.obj2BVH(triangleList, glm::vec3(0.725f, 0.71f, 0.68f));
	ourModel3.obj2BVH(triangleList, glm::vec3(0.725f, 0.71f, 0.68f));
	ourModel4.obj2BVH(triangleList, glm::vec3(0.63f, 0.065f, 0.05f));
	ourModel5.obj2BVH(triangleList, glm::vec3(0.14f, 0.45f, 0.091f));
	ourModel6.obj2BVH(triangleList,glm::vec3(0.65f),true);
	BVH* firstBVH=recursionBVH(triangleList, 0, triangleList.size(), xAxis);
	float* bvh = NULL;
	float* tri = NULL;
	int bvhSize, triSize;
	createTexture(firstBVH, bvh, tri, bvhSize, triSize);
	Ray *myRay = new Ray(SCR_WIDTH, SCR_HEIGHT, camera, ourShader, bvh, tri, bvhSize, triSize);
	int i = 0;
	int spp = 20;
	frame2Real f=frame2Real(SCR_WIDTH,SCR_HEIGHT, spp);
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ourShader.use();
		myRay->Draw(ourShader);
		glfwSwapBuffers(window);
		if (i != 0) {
			std::cout << i << std::endl;
			f.frame2local();
		}
		if (i++ == spp) {
			break;
		}
	}
	f.saveFrameBuff("hello.bmp",SCR_WIDTH,SCR_HEIGHT);
	glfwTerminate();
	return 0;
}
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}