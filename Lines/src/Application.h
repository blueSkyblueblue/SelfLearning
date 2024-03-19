#pragma once
#include <glm/glm.hpp>
#include "Window.h"
#include "Shader.h"

class Camera
{


private:
	float x, y, z;
};


class Application
{
public:
	Application();
	~Application();
	inline static Application* GetApp() { return s_App; }

	void setup();
	void run();
public:
	static void OnKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void OnMouseButton(GLFWwindow* window, int button, int action, int mods);
	static void OnCursorPos(GLFWwindow* window, double xPos, double yPos);
	static void OnFramebufferResize(GLFWwindow* window, int width,  int height);

	void processInput();
private:
	Window* m_Window;
	Shader* m_Shader;

	GLFWcursor* cursor;
private:
	glm::mat4 m_Camera;
	glm::mat4 m_Rotate;
					
	glm::mat4 m_Pers;
	glm::mat4 m_MVP;
private:
	bool m_Running;

	static Application* s_App;
};					
