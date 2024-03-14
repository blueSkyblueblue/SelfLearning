#pragma once
#include <GLFW/glfw3.h>
#include <string>

class Window
{
public:
	Window(int32_t width, int32_t height, const std::string& title);
	~Window();

	void makeContexCurrent() const;
	void swapBuffers() const;

	bool shouldClose() const;

	inline GLFWwindow* getInstance() { return m_Window; }
private:
	void setup();
private:
	int32_t m_Width, m_Height;
	std::string m_Title;

	GLFWwindow* m_Window;
};

