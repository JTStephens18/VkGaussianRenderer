#pragma once

#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vr {

	class VrWindow {

	public:
		VrWindow(int w, int h, std::string name);
		~VrWindow();

		VrWindow(const VrWindow&) = delete;
		VrWindow& operator = (const VrWindow&) = delete;

		bool shouldClose() {
			return glfwWindowShouldClose(window);
		}

		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		bool wasWindowResized() { return frameBufferResized; }
		void resetWindowResizedFlag() { frameBufferResized = false; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:

		static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();

		int width;
		int height;
		bool frameBufferResized = false;

		std::string windowName;

		GLFWwindow* window;

};
}