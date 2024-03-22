#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "Application.h"
#include "Logger/Logger.h"


constexpr int32_t INITIAL_WIDTH = 1600;
constexpr int32_t INITIAL_HEIGHT = 1200;

#define APP_ASSERT(expression, ...) if (!(expression)) { LOG_ERROR(__VA_ARGS__); __debugbreak(); }

static void glfw_error_callback(int errorCode, const char* description);

Application* Application::s_App = new Application();

Application::Application()
	: m_Running{ false },
	m_Camera {glm::identity<glm::mat4>()},
	m_Rotate {glm::identity<glm::mat4>()},
	m_Direction {glm::identity<glm::mat4>()},
	m_Pers { glm::perspective(glm::radians(75.f), (float)INITIAL_WIDTH / INITIAL_HEIGHT, 0.1f, 1000.f)},
	m_MVP {glm::identity<glm::mat4>()},
	m_VerticalRadian {0.f},
	m_HorizontalRadian {0.f},
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

	glDeleteBuffers(1, &m_Box);
	glDeleteBuffers(1, &m_BoxBuffer);
	glDeleteBuffers(1, &m_BoxIndicesBuffer);

	glDeleteBuffers(1, &m_Floor);
	glDeleteBuffers(1, &m_FloorBuffer);
	glDeleteBuffers(1, &m_FloorIndicesBuffer);

	delete m_Window;
	delete m_Shader;

	delete s_App;
}

void Application::setup()
{
	glfwSetWindowUserPointer(m_Window->getInstance(), this);

	cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	glfwSetCursor(m_Window->getInstance(), cursor);

	glfwSetWindowCloseCallback(m_Window->getInstance(), OnWindowClose);
	glfwSetKeyCallback(m_Window->getInstance(), OnKeyPressed);
	glfwSetMouseButtonCallback(m_Window->getInstance(), OnMouseButton);
	glfwSetCursorPosCallback(m_Window->getInstance(), OnCursorPos);
	glfwSetFramebufferSizeCallback(m_Window->getInstance(), OnFramebufferResize);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("res/fonts/bahnschrift.ttf", 24.0f);

	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_Window->getInstance(), true);
	ImGui_ImplOpenGL3_Init("#version 330");

	m_Shader = new Shader("res/shaders/shader.vs", "res/shaders/shader.fs");
	m_Shader->bind();

	int32_t width, height;
	glfwGetFramebufferSize(m_Window->getInstance(), &width, &height);
	m_Pers = glm::perspective(glm::radians(75.0f), (float)width / height, 0.1f, 100.0f);
	m_Camera = glm::translate(m_Camera, glm::vec3(0.f, 0.f, -10.f));
	
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	prepareData();
}

void Application::prepareData()
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

	glGenVertexArrays(1, &m_Box);
	glGenBuffers(1, &m_BoxBuffer);
	glGenBuffers(1, &m_BoxIndicesBuffer);

	glBindVertexArray(m_Box);

	glBindBuffer(GL_ARRAY_BUFFER, m_BoxBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BoxIndicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boxIndices), boxIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	glfwSwapInterval(1);

	glGenVertexArrays(1, &m_Floor);
	glGenBuffers(1, &m_FloorBuffer);
	glGenBuffers(1, &m_FloorIndicesBuffer);

	float m_FloorVertices[] = {
		-1.0f, -5.0f, 0.0f,		 .7f, .7f, .7f,
		 1.0f, -5.0f, 0.0f,		 .7f, .7f, .7f
	};

	uint32_t m_FloorIndices[] = {
		0, 1
	};

	glBindVertexArray(m_Floor);
	glBindBuffer(GL_ARRAY_BUFFER, m_FloorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_FloorVertices), m_FloorVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_FloorIndicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_FloorIndices), m_FloorIndices, GL_STATIC_DRAW);

	APP_ASSERT(glGetError() == GL_NO_ERROR, "There are some errors: {}", glGetError());
}

void Application::run()
{
	glm::mat4 model = glm::identity<glm::mat4>();

	bool show_demo_window = true;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	m_Running = true;
	while (m_Running)
	{
		processInput();

		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiIO& io = ImGui::GetIO();
		{
			ImGui::Begin("Testing!");                         
			ImGui::Text("Background Color: ");                      
			ImGui::ColorEdit3("clear color", (float*)&clear_color);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		static float rotateAngleX = 23.f;// = (float)glfwGetTime();
		static float rotateAngleY = 54.f;// = (float)glfwGetTime();
		static float rotateAngleZ = 23.f;// = (float)glfwGetTime();
		{
			ImGui::Begin("Draw Lines");
			ImGui::Text("Set the object rotation angle in degrees");
			ImGui::SliderFloat("X Rotation", &rotateAngleX, 0.0f, 360.f);
			ImGui::Text("Set the object rotation angle in degrees");
			ImGui::SliderFloat("Y Rotation", &rotateAngleY, 0.0f, 360.f);
			ImGui::Text("Set the object rotation angle in degrees");
			ImGui::SliderFloat("Z Rotation", &rotateAngleZ, 0.0f, 360.f);
			ImGui::End();
		}


		m_Rotate = glm::rotate(glm::mat4(1.f), glm::radians(m_VerticalRadian), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(m_HorizontalRadian), glm::vec3(0.f, 1.f, 0.f));
		m_MVP = m_Pers * m_Rotate * m_Camera;
		m_Shader->setUniformMat4("u_MVP", m_MVP);

		glBindVertexArray(m_Box);
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(rotateAngleX), glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotateAngleY), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotateAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));
		m_Shader->setUniformMat4("u_Model", rotate);
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(m_Floor);
		m_Shader->setUniformMat4("u_Model", model);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(2.f);
		for (int i = -20; i != 20; i++)
		{
			model = glm::mat4(1.f);
			model = glm::scale(model, glm::vec3(20.f, 1.0f, 1.0f));
			model = glm::translate(model, glm::vec3(0.f, 0.f,(float)i));
			m_Shader->setUniformMat4("u_Model", model);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
		}

		for (int i = -20; i != 20; i++)
		{
			model = glm::mat4(1.f);
			model = glm::scale(model, glm::vec3(1.f, 1.f, 20.f));
			model = glm::rotate(model, glm::radians((float)90), glm::vec3(0.f, 1.f, 0.f));
			model = glm::translate(model, glm::vec3(0.f, 0.f, (float)i));
			m_Shader->setUniformMat4("u_Model", model);
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
		}

		APP_ASSERT(glGetError() == GL_NO_ERROR, "There are some errors!");

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		m_Window->swapBuffers();
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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

void Application::OnWindowClose(GLFWwindow* window)
{
	Application* app = (Application*)glfwGetWindowUserPointer(window);
	app->m_Running = false;
}

void Application::OnKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static bool isFullScreen = false;
	Application* app = ((Application*)glfwGetWindowUserPointer(window))->GetApp();

	if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
	switch (key)
	{
		case GLFW_KEY_ESCAPE:
		{
			app->m_Running = false;
			break;
		}
		case GLFW_KEY_F:
		{
			if (mods != GLFW_MOD_CONTROL) break;

			if (isFullScreen)
			{
				glfwSetWindowMonitor(window, nullptr, 300, 200, INITIAL_WIDTH, INITIAL_HEIGHT, 0);
			}
			else
			{
				GLFWmonitor* monitor = glfwGetPrimaryMonitor();
				const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
				glfwSetWindowMonitor(window, monitor, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
			}

			isFullScreen = !isFullScreen;
			break;
		}
	}
}

void Application::OnMouseButton(GLFWwindow* window, int button, int action, int mods)
{
	LOG_INFO("Button Event: [button:{0}, action:{1}, mods:{2}]", button, action, mods);

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
	static float sencitiveness = 0.05f;
	static float recordPosX = 0.f;
	static float recordPosY = 0.f;
	static Application* app = (Application*)glfwGetWindowUserPointer(window);
	
	int mouseLeftButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

	switch (mouseLeftButtonState)
	{
		case GLFW_PRESS:
		{
			float xOffset = (float)xPos - recordPosX;
			float yOffset = (float)yPos - recordPosY;

			app->m_HorizontalRadian -= xOffset * sencitiveness;
			app->m_VerticalRadian -= yOffset * sencitiveness;

			break;
		}
		case GLFW_RELEASE:
		{
			break;
		}
	}

	recordPosX = (float)xPos;
	recordPosY = (float)yPos;
}

void Application::OnFramebufferResize(GLFWwindow* window, int width, int height)
{
	Application* app = (Application*)glfwGetWindowUserPointer(window);
	glViewport(0, 0, width, height);
	if (width == 0 || height == 0) return;

	app->m_Pers = glm::perspective(glm::radians(75.f), (float)width / height, 0.1f, 1000.f);
}

void Application::processInput()
{
	processKey();
}

void Application::processKey()
{
	static constexpr int keys[6] = {
	GLFW_KEY_A, GLFW_KEY_D,
	GLFW_KEY_W, GLFW_KEY_X,
	GLFW_KEY_J, GLFW_KEY_K
	};

	static constexpr float transitionSpeed = 0.07f;
	static glm::vec3 direction;

	for (int key : keys)
	{
		if (glfwGetKey(m_Window->getInstance(), key) != GLFW_PRESS) continue;

		switch (key)
		{
		case GLFW_KEY_A:
		{
			LOG_INFO("KEY_A is Pressed");
			direction =/*/* glm::mat3(m_Direction) * */glm::vec3(transitionSpeed, 0.f, 0.f);
			m_Camera = glm::translate(m_Camera, direction);
			break;
		}
		case GLFW_KEY_D:
		{
			LOG_INFO("KEY_D is Pressed");
			direction =/*/* glm::mat3(m_Direction) * */glm::vec3(-transitionSpeed, 0.f, 0.f);
			m_Camera = glm::translate(m_Camera, direction);
			break;
		}
		case GLFW_KEY_W:
		{
			LOG_INFO("KEY_W is Pressed");
			direction =/*/* glm::mat3(m_Direction) * */glm::vec3(0.f, 0.f, transitionSpeed);
			m_Camera = glm::translate(m_Camera, direction);
			break;
		}
		case GLFW_KEY_X:
		{
			LOG_INFO("KEY_X is Pressed");
			direction =/*/* glm::mat3(m_Direction) * */glm::vec3(0.f, 0.f, -transitionSpeed);
			m_Camera = glm::translate(m_Camera, direction);
			break;
		}
		case GLFW_KEY_J:
		{
			LOG_INFO("KEY_J is Pressed");
			direction =/* glm::mat3(m_Direction) * */glm::vec3(0.f, transitionSpeed, 0.f);
			m_Camera = glm::translate(m_Camera, direction);
			break;
		}
		case GLFW_KEY_K:
		{
			LOG_INFO("KEY_K is Pressed");
			direction =/* glm::mat3(m_Direction) * */glm::vec3(0.f, -transitionSpeed, 0.f);
			m_Camera = glm::translate(m_Camera, direction);
			break;
		}
		}
	}

}

void Application::processCursor()
{
}
