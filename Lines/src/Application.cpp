#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <iostream>

constexpr int32_t INITIAL_WIDTH = 800;
constexpr int32_t INITIAL_HEIGHT = 600;

static void glfw_error_callback(int errorCode, const char* description)
{

}

int main()
{
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW!" << std::endl;
		return 0;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "Test premake", nullptr, nullptr);
	if (!window)
	{
		std::cout << "Create Window Failed!" << std::endl;
		glfwTerminate();
		return 0;
	}

	while (!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}