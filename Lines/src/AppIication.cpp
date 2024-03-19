#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Window.h"
#include "Shader.h"

#include "Application.h"

#include "Logger/Logger.h"


constexpr int32_t INITIAL_WIDTH = 1600;
constexpr int32_t INITIAL_HEIGHT = 1200;

#define APP_ASSERT(expression, ...) if (!(expression)) { LOG_ERROR(__VA_ARGS__); __debugbreak(); }

static void glfw_error_callback(int errorCode, const char* description);

Application* Application::s_App = new Application();

Application::Application()
	: m_Running{ false }, 
	m_CameraPosX {0.0f},
	m_CameraPosY {0.0f},
	m_CameraPosZ {0.0f},
	m_Rotate {glm::identity<glm::mat4>()},
	m_MVP {glm::identity<glm::mat4>()},
	cursor {nullptr}
{
	m_Window = new Window(INITIAL_WIDTH, INITIAL_HEIGHT, "Draw Lines");
	m_Window->makeContexCurrent();

	APP_ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to initialize Glad");
	setup();
}

Application::~Application()
{
	glfwDestroyCursor(cursor);

	delete m_Window;
	delete m_Shader;

	delete s_App;
}

void Application::setup()
{
	glfwSetWindowUserPointer(m_Window->getInstance(), this);

	cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	glfwSetCursor(m_Window->getInstance(), cursor);

	glfwSetKeyCallback(m_Window->getInstance(), OnKeyPressed);
	glfwSetMouseButtonCallback(m_Window->getInstance(), OnMouseMove);
	glfwSetCursorPosCallback(m_Window->getInstance(), OnCursorPos);
	glfwSetFramebufferSizeCallback(m_Window->getInstance(), OnFramebufferResize);

	m_Shader = new Shader("res/shaders/shader.vs", "res/shaders/shader.fs");
	m_Shader->bind();

	int32_t width, height;
	glfwGetFramebufferSize(m_Window->getInstance(), &width, &height);
	m_Pers = glm::perspective(glm::radians(75.0f), (float)width / height, 0.1f, 1000.0f);
}

void Application::run()
{
	float boxVertices[] = {
		// Positions			// Colors
		-1.0f, -1.0f, 1.0f,		0.4f, 0.5f, 0.3f,
		 1.0f, -1.0f, 1.0f,		0.4f, 0.5f, 0.3f,
		 1.0f,  1.0f, 1.0f,		0.4f, 0.5f, 0.3f,
		-1.0f,  1.0f, 1.0f,		0.4f, 0.5f, 0.3f,

		-1.0f, -1.0f, -1.0f,	0.2f, 0.8f, 0.5f,
		 1.0f, -1.0f, -1.0f,	0.2f, 0.8f, 0.5f,
		 1.0f,  1.0f, -1.0f,	0.2f, 0.8f, 0.5f,
		-1.0f,  1.0f, -1.0f,	0.2f, 0.8f, 0.5f
	};

	uint32_t boxIndices[] = {
		0, 1, 2,	0, 2, 3,
		4, 5, 6,	4, 6, 7
	};

	uint32_t vao, vbo, ibo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boxIndices), boxIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));


	//uint32_t floor, floorBuffer, floorElems;
	//glGenVertexArrays(1, &floor);
	//glGenBuffers(1, &floorBuffer);
	//glGenBuffers(1, &floorElems);

	//glBindVertexArray(floor);
	//float floorVertices[] = {
	//	-1.0f, -1.0f, 0.0f,		 1.0f, 1.0f, 1.0f,
	//	 1.0f, -1.0f, 0.0f,		 1.0f, 1.0f, 1.0f
	//};

	//uint32_t floorIndices[] = {
	//	1, 2
	//};

	/*glBindBuffer(GL_ARRAY_BUFFER, floorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorElems);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);
	*/

	m_Running = true;
	while (m_Running)
	{
		processInput();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float time = (float)glfwGetTime();
		m_Camera = glm::translate(glm::mat4(1.0f), glm::vec3(m_CameraPosX, m_CameraPosY, -20.0f + m_CameraPosZ));
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), time, glm::vec3(0.0f, 1.0f, 1.0f));
		m_MVP = m_Pers * m_Rotate * m_Camera * rotate;

		m_Shader->setUniformMat4("u_MVP", m_MVP);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr);

		//glBindVertexArray(floor);
		//glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);

		APP_ASSERT(glGetError() == GL_NO_ERROR, "There are some errors!");

		m_Window->swapBuffers();
		glfwPollEvents();
	}
}

int main()
{
	Log::Init();
	glfwSetErrorCallback(glfw_error_callback);
	APP_ASSERT(glfwInit(), "Failed to initialize GLFW!");

	LOG_INFO("The Current Version of OpenGL : {0}", (char*)glGetString(GL_VERSION));

	Application* app = Application::GetApp();

	app->run();

	glfwTerminate();
}

static void glfw_error_callback(int errorCode, const char* description)
{
	LOG_ERROR("ERROR CODE: {0}", errorCode);
	LOG_TRACE("DESCRIPTION\n\t {0}", description);
}

void Application::OnKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Application* app = ((Application*)glfwGetWindowUserPointer(window))->GetApp();

	if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

	switch (key)
	{
		case GLFW_KEY_ESCAPE:
		{
			app->m_Running = false;
			break;
		}
	}
}

void Application::OnMouseMove(GLFWwindow* window, int button, int action, int mods)
{
	switch (button)
	{
	case GLFW_PRESS:
	{

	}
	case GLFW_RELEASE:
	{

	}
	}
}

void Application::OnCursorPos(GLFWwindow* window, double xPos, double yPos)
{
	static float sencitiveness = 0.0008f;

	Application* app = (Application*)glfwGetWindowUserPointer(window);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float xOffset = (float)xPos - (float)width / 2;
	float yOffset = (float)yPos - (float)height / 2;

	spdlog::default_logger()->info("----- Cursor Position: [{0}, {1}] -------", xPos, yPos);

	if (xOffset || yOffset)
	{
		LOG_WARN("The Cursor Position Offsets: ({0}, {1})", xOffset, yOffset);

		app->m_Rotate *= glm::rotate(glm::mat4(1.f), yOffset * sencitiveness, {1.f, 0.f, 0.f}) *
			glm::rotate(glm::mat4(1.f), xOffset * sencitiveness, {0.f, 1.f, 0.f});

		glfwSetCursorPos(window, (double)width / 2, (double)height / 2);
	}
}

void Application::OnFramebufferResize(GLFWwindow* window, int width, int height)
{
	Application* app = (Application*)glfwGetWindowUserPointer(window);
	glViewport(0, 0, width, height);
	app->m_Pers = glm::perspective(glm::radians(75.f), (float)width / height, 0.1f, 1000.f);
}

void Application::processInput()
{
	static constexpr int keys[6] = {
		GLFW_KEY_A, GLFW_KEY_D,
		GLFW_KEY_W, GLFW_KEY_X,
		GLFW_KEY_J, GLFW_KEY_K
	};

	static constexpr float transitionSpeed = 0.005f;

	for (int key : keys)
	{
		if (glfwGetKey(m_Window->getInstance(), key) != GLFW_PRESS) continue;

		switch (key)
		{
			case GLFW_KEY_A:
			{
				LOG_INFO("KEY_A is Pressed");
				m_CameraPosX -= transitionSpeed;
				break;
			}
			case GLFW_KEY_D:
			{
				LOG_INFO("KEY_D is Pressed");
				m_CameraPosX += transitionSpeed;
				break;
			}
			case GLFW_KEY_W:
			{
				LOG_INFO("KEY_W is Pressed");
				m_CameraPosY += transitionSpeed;
				break;
			}
			case GLFW_KEY_X:
			{
				LOG_INFO("KEY_X is Pressed");
				m_CameraPosY -= transitionSpeed;
				break;
			}
			case GLFW_KEY_J:
			{
				LOG_INFO("KEY_J is Pressed");
				m_CameraPosZ += transitionSpeed;
				break;
			}
			case GLFW_KEY_K:
			{
				LOG_INFO("KEY_K is Pressed");
				m_CameraPosZ -= transitionSpeed;
				break;
			}
		}
	}
}
