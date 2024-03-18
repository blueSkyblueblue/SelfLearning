#include <glad/glad.h>
//#include <glm/gtx/transform.hpp>

#include "Window.h"
#include "Shader.h"

#include "Logger/Logger.h"

constexpr int32_t INITIAL_WIDTH = 800;
constexpr int32_t INITIAL_HEIGHT = 600;

#define APP_ASSERT(expression, ...) if (!(expression)) { LOG_ERROR(__VA_ARGS__); __debugbreak(); }

static void glfw_error_callback(int errorCode, const char* description);

int main()
{
	Log::Init();
	glfwSetErrorCallback(glfw_error_callback);
	APP_ASSERT(glfwInit(), "Failed to initialize GLFW!");

	Window* window = new Window(INITIAL_WIDTH, INITIAL_HEIGHT, "Draw Lines");
	window->makeContexCurrent();
	APP_ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to initialize Glad");

	LOG_INFO("The Current Version of OpenGL : {0}", (char*)glGetString(GL_VERSION));

	uint32_t vao = {};
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	float vertices[] = {
		-0.8f, -0.0f, 0.0f,
		 0.8f, -0.0f, 0.0f,
		 0.0f, -0.8f, 0.0f,
		 0.0f,  0.8f, 0.0f
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

	glEnable(GL_LINE_SMOOTH);

	float initialLineWidth;
	glGetFloatv(GL_LINE_WIDTH, &initialLineWidth);
	LOG_TRACE("{}", initialLineWidth);

	// Create Shader and Bind it;
	Shader basicShader("res/shaders/shader.vs", "res/shaders/shader.fs");
	basicShader.bind();

	// Set user optional data, and bind it to current window
	struct WindowUserInfo
	{
		int32_t lineCount;
		float linespace;
		std::pair<float, float> drag;
		std::pair<int32_t, int32_t> windowPos;

		WindowUserInfo() : lineCount { 10 }, linespace { 0.1f }, windowPos {} {}
		~WindowUserInfo() = default;
	};

	WindowUserInfo userInfo = {};
	glfwSetWindowUserPointer(window->getInstance(), &userInfo);

	// Set a series of callback functions (handling events)
	glfwSetScrollCallback(window->getInstance(), [](GLFWwindow* window, double xoffset, double yoffset)
		{
			static float& linespace = reinterpret_cast<WindowUserInfo*>(glfwGetWindowUserPointer(window))->linespace;
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			linespace *= ((float)yoffset * 50 + (float)height) / (float)height;

			LOG_INFO("Mouse Scroll: {0}", yoffset);
		});

	glfwSetCursorPosCallback(window->getInstance(), [](GLFWwindow* window, double xpos, double ypos)
		{
			static WindowUserInfo* userInfo = reinterpret_cast<WindowUserInfo*>(glfwGetWindowUserPointer(window));
			static std::pair<float, float>& drag = userInfo->drag;

			static bool needReset = true;
			static float previousX = (float)xpos;
			static float previousY = (float)ypos;
			static std::pair<float, float> dragCache = { 0.0f, 0.0f };

			if (needReset)
			{
				previousX = (float)xpos;
				previousY = (float)ypos;
				needReset = false;
			}
			
			int32_t action = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
			if (action == GLFW_PRESS)
			{
				int32_t width, height;
				glfwGetFramebufferSize(window, &width, &height);

				drag.first = dragCache.first + (float)(xpos - previousX) / width;
				drag.second = dragCache.second - (float)(ypos - previousY) / height;

				LOG_INFO("Mouse Drag: {0}, {1}", drag.first, drag.second);
			}
			else if (action == GLFW_RELEASE)
			{
				needReset = true;
				dragCache = { drag.first, drag.second };

				LOG_INFO("Mouse button released...");
			}
		});



	// Main Loop;
	basicShader.setUnifrom3f("u_Color", 0.8f, 0.7f, 0.8f);
	float& linespace = userInfo.linespace;
	auto& [xDragged, yDragged] = userInfo.drag;
	while (!window->shouldClose())
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		basicShader.setUnifrom2f("u_Dragged", xDragged, yDragged);

		// Draw lines on screen
		basicShader.setUnifrom2f("u_Transition", 0, 0);
		basicShader.setUnifrom3f("u_Color", 1.0f, 1.0f, 1.0f);
		glLineWidth(2.5f);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[0]);
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[1]);
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
		for (float offset = linespace; offset <= 0.8f; offset += linespace)
		{
			if ((int)std::round(offset / linespace) % 5 == 0)
			{
				basicShader.setUnifrom3f("u_Color", 0.5f, 0.6f, 0.8f);
				glLineWidth(1.3f);
			}
			else
			{
				basicShader.setUnifrom3f("u_Color", 0.6f, 0.6f, 0.6f);
				glLineWidth(0.8f);
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[0]);
			
			basicShader.setUnifrom2f("u_Transition", 0, offset);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
			basicShader.setUnifrom2f("u_Transition", 0, -offset);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[1]);
			
			basicShader.setUnifrom2f("u_Transition", offset, 0);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
			basicShader.setUnifrom2f("u_Transition", -offset, 0);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
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

static void glfw_error_callback(int errorCode, const char* description)
{
	LOG_ERROR("ERROR CODE: {0}", errorCode);
	LOG_TRACE("DESCRIPTION\n\t {0}", description);
}
