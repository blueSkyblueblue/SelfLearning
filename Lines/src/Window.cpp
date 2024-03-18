#include "Window.h"
#include "Logger/Logger.h"

#include <glad/glad.h>


Window::Window(int32_t width, int32_t height, const std::string& title)
	: m_Width { width }, m_Height { height }, m_Title { title }, m_Window { nullptr }
{
	if (!glfwInit()) // make sure glfw is already initialized.
	{
		LOG_ERROR("Failed to initialize GLFW!");
		__debugbreak();
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

	m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!m_Window)
	{
		LOG_ERROR("Create Window Failed!");
		__debugbreak();
	}

	this->setup();
	this->makeContexCurrent(); // Make opengl context current, waiting for drawing

	glfwSetWindowUserPointer(m_Window, this);
}

Window::~Window()
{
	if (!m_Window)
	{
		LOG_ERROR("The window is not valid!");
	}

	glfwDestroyWindow(m_Window);
}

void Window::makeContexCurrent() const
{
	glfwMakeContextCurrent(m_Window);
}

void Window::swapBuffers() const
{
	glfwSwapBuffers(m_Window);
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(m_Window);
}

void Window::setup()
{
	glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int32_t width, int32_t height)
		{
			glViewport(0, 0, width, height);
		});

	glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
		{
			if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
				glfwSetWindowShouldClose(window, GLFW_TRUE);
		});
}
