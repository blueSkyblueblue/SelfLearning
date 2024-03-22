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
	void prepareData();
	void run();
public:
	static void OnWindowClose(GLFWwindow* window);
	static void OnKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void OnMouseButton(GLFWwindow* window, int button, int action, int mods);
	static void OnCursorPos(GLFWwindow* window, double xPos, double yPos);
	static void OnFramebufferResize(GLFWwindow* window, int width,  int height);

	void processInput();
	void processKey();
	void processCursor();
private:
	Window* m_Window;
	Shader* m_Shader;

	GLFWcursor* cursor;
private:
	uint32_t m_Box, m_BoxBuffer, m_BoxIndicesBuffer;
	uint32_t m_Floor, m_FloorBuffer, m_FloorIndicesBuffer;

	glm::mat4 m_Camera;
	glm::mat4 m_Rotate;
	glm::mat4 m_Direction;

	float m_VerticalRadian;
	float m_HorizontalRadian;
					
	glm::mat4 m_Pers;
	glm::mat4 m_MVP;
private:
	bool m_Running;

	static Application* s_App;
};					
