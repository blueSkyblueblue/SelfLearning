#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <iostream>
#include <string>

#include "Logger/Logger.h"

constexpr int32_t INITIAL_WIDTH = 800;
constexpr int32_t INITIAL_HEIGHT = 600;

static void glfw_error_callback(int errorCode, const char* description)
{
	LOG_ERROR("ERROR CODE: {0}", errorCode);
	LOG_TRACE("DESCRIPTION\n\t {0}", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void frame_size_resize_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
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
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, frame_size_resize_callback);

	glfwMakeContextCurrent(window);
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
		 1.0f, -0.0f, 0.0f
	};

	uint32_t vbo = {};
	glGenBuffers(1, &vbo);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, vertices, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	uint32_t indices[] = {
		0, 1
	};

	uint32_t ibo = {};
	glGenBuffers(1, &ibo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 2, indices, GL_STATIC_DRAW);

	uint32_t hasError = glGetError();
	if (hasError != GL_NO_ERROR) LOG_ERROR("There is an error, and error code : {}", hasError);
	
	std::string vertexShaderSrc = R"(
		#version 330 core
		
		layout(location = 0) in vec3 a_Position;		

		void main()
		{
			gl_Position = vec4(a_Position, 1.0);
		}
	)";

	std::string fragmentShaderSrc = R"(
		#version 330 core
		
		out vec4 fragColor;

		void main()
		{
			fragColor = vec4(1.0, 1.0, 1.0, 1.0);
		}
	)";

	uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* shaderSrc = vertexShaderSrc.c_str();
	glShaderSource(vs, 1, &shaderSrc, nullptr);
	glCompileShader(vs);
	int32_t result = {};
	glGetShaderiv(vs, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		int32_t length = {};
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &length);

		GLchar* infoLog = new GLchar[length];
		glGetShaderInfoLog(vs, length, &length, infoLog);

		LOG_ERROR("Failed to compile vertex shader: \n \t{0}", infoLog);

		delete[] infoLog;
	}

	uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);
	shaderSrc = fragmentShaderSrc.c_str();
	glShaderSource(fs, 1, &shaderSrc, nullptr);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		int32_t length = {};
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &length);

		GLchar* infoLog = new GLchar[length];
		glGetShaderInfoLog(fs, length, &length, infoLog);

		LOG_ERROR("Failed to compile fragment shader: \n \t{0}", infoLog);

		delete[] infoLog;
	}

	uint32_t shader = glCreateProgram();
	glAttachShader(shader, vs);
	glAttachShader(shader, fs);
	glLinkProgram(shader);

	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if (!result)
	{
		int32_t length = {};
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);

		GLchar* infoLog = new GLchar[length];
		glGetProgramInfoLog(shader, length, &length, infoLog);

		LOG_ERROR("Failed to link program: \n \t{0}", infoLog);

		delete[] infoLog;
	}
	
	glDetachShader(shader, fs);
	glDetachShader(shader, vs);
	glDeleteShader(fs);
	glDeleteShader(vs);

	glValidateProgram(shader);

	glUseProgram(shader);
	while (!glfwWindowShouldClose(window))
	{

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);		

		if (glGetError() != GL_NO_ERROR)
			LOG_ERROR("There are some errors!");

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}