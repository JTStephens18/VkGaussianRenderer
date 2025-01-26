#include "vr_window.hpp"

#include <stdexcept>

namespace vr {
	VrWindow::VrWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
		initWindow();
	}

	VrWindow::~VrWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void VrWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
	}

	void VrWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
	}

	void VrWindow::frameBufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto vrWindow = reinterpret_cast<VrWindow*>(glfwGetWindowUserPointer(window));
		vrWindow->frameBufferResized = true;
		vrWindow->width = width;
		vrWindow->height = height;
	}
}