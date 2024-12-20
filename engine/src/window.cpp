#include "window.hpp"

#include <GLFW/glfw3.h>

void Window::SetName(const std::string &name) {
    glfwSetWindowTitle(glfwGetCurrentContext(), name.c_str());
}