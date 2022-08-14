#pragma once

#include <GLFW/glfw3.h>

#include <string>

namespace Vipera
{
	class Window
	{
		int width;
		int height;

		std::string name;

		GLFWwindow* window;

	public:
		Window(int width, int height, std::string name);
		~Window();

	private:
		void initWindow();
	};
}