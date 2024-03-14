#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Window.h"
#include "Shader.h"

#include <iostream>

#include "Logger/Logger.h"

constexpr int32_t INITIAL_WIDTH = 800;
constexpr int32_t INITIAL_HEIGHT = 600;

static void glfw_error_callback(int errorCode, const char* description)
{
	LOG_ERROR("ERROR CODE: {0}", errorCode);
	LOG_TRACE("DESCRIPTION\n\t {0}", description);
}

int main()
{
	Log::Init();
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW!" << std::endl;
		return -1;
	}

	Window* window = new Window(INITIAL_WIDTH, INITIAL_HEIGHT, "Draw Lines");

	window->makeContexCurrent();
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize Glad" << std::endl;
		return -1;
	}

	LOG_INFO("The Current Version of OpenGL : {0}", (char*)glGetString(GL_VERSION));

	uint32_t vao = {};
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	float vertices[] = {
		-1.0f, -0.0f, 0.0f,
		 1.0f, -0.0f, 0.0f,
		 0.0f, -1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f
	};

	uint32_t vbo = {};
	glGenBuffers(1, &vbo);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vertices, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	uint32_t indices[] = {
		0, 1,
		2, 3
	};

	uint32_t ibo[2] = {};
	glGenBuffers(2, ibo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 2, indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 2, &indices[0] + 2, GL_STATIC_DRAW);

	uint32_t hasError = glGetError();
	if (hasError != GL_NO_ERROR) LOG_ERROR("There is an error, and error code : {}", hasError);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2);

	// Create Shader and Bind it;
	Shader basicShader("res/shaders/shader.vs", "res/shaders/shader.fs");
	basicShader.bind();


	// Set user optional data, and bind it to current window
	struct WindowUserInfo
	{
		int32_t lineCount;
		std::pair<float, float> drag;
		std::pair<int32_t, int32_t> windowPos;

		WindowUserInfo() : lineCount { 10 }, windowPos {} {}
		~WindowUserInfo() = default;
	};

	WindowUserInfo userInfo = {};
	glfwSetWindowUserPointer(window->getInstance(), &userInfo);

	// Set a series of callback function (handling events)
	glfwSetScrollCallback(window->getInstance(), [](GLFWwindow* window, double xoffset, double yoffset)
		{
			static WindowUserInfo* userInfo = reinterpret_cast<WindowUserInfo*>(glfwGetWindowUserPointer(window));
			static int32_t& lineCount = userInfo->lineCount;

			LOG_INFO("The mouse event => (xoffset: {0}, yoffset: {1})", xoffset, yoffset);

			if (lineCount <= 1 && yoffset > 0)
			{
				lineCount = 1;
				return;
			}

			lineCount -= (int)std::round(yoffset);
		});

	glfwSetCursorPosCallback(window->getInstance(), [](GLFWwindow* window, double xpos, double ypos)
		{
			static WindowUserInfo* userInfo = reinterpret_cast<WindowUserInfo*>(glfwGetWindowUserPointer(window));
			static std::pair<float, float>& drag = userInfo->drag;

			static bool needReset = true;
			static float previousX = (float)xpos;
			static float previousY = (float)xpos;
			if (needReset)
			{
				previousX = (float)xpos;
				previousY = (float)xpos;
				needReset = false;
			}

			int32_t action = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
			if (action == GLFW_PRESS)
			{
				int32_t width, height;
				glfwGetFramebufferSize(window, &width, &height);

				drag.first = (drag.first - previousX) / (float)width;
				drag.second = (drag.second - previousY) / (float)height;

				LOG_INFO("Mouse Drag: {0}, {1}", drag.first, drag.second);
			}
			else if (action == GLFW_RELEASE)
			{
				needReset = true;
			}
		});

	// Main Loop;
	while (!window->shouldClose())
	{
		// processing inputs

		// ==================

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		// Draw lines on screen
		{
			for (float i = -1.0f; i <= 1.0f; i += 2.0f / userInfo.lineCount)
			{
				if ((int)(i * userInfo.lineCount) % 5 == 0)
				{
					basicShader.setUnifrom3f("u_Color", 0.3f, 0.5f, 0.6f);
				}
				else basicShader.setUnifrom3f("u_Color", 0.8f, 0.6f, 0.4f);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[0]);
				basicShader.setUnifrom2f("u_Transition", 0, i);
				glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
			}

			for (float i = -1.0f; i <= 1.0f; i += 2.0f / userInfo.lineCount)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[1]);
				basicShader.setUnifrom2f("u_Transition", i, 0);
				glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
			}
		}

		if (glGetError() != GL_NO_ERROR)
			LOG_ERROR("There are some errors!");

		window->swapBuffers();
		glfwPollEvents();
	}

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(2, ibo);
	
	glDeleteVertexArrays(1, &vao);

	delete window;
	glfwTerminate();
}