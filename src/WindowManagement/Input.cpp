#include "Input.hpp"

#include "vendor/glfw/include/GLFW/glfw3.h"

namespace src::windowmanagement
{
auto Input::setKeyCallback(const KeyCallback& callback) -> void
{
    keyCallback_ = callback;
}

auto Input::setWindowSizeCallback(const WindowSizeCallback& callback) -> void
{
    windowSizeCallback_ = callback;
}

auto Input::bindWindow(GLFWwindow* windowHandle) -> void
{
    glfwSetWindowUserPointer(windowHandle, this);

    glfwSetKeyCallback(windowHandle,
        [](GLFWwindow* win, int32_t key, int32_t scanCode, int32_t action, int32_t mods)
        {
            const Input* input = static_cast<Input*>(glfwGetWindowUserPointer(win));
            input->keyCallback_(key, scanCode, action, mods);
        });

    glfwSetWindowSizeCallback(windowHandle,
        [](GLFWwindow* win, int32_t x, int32_t y)
        {
            const Input* input = static_cast<Input*>(glfwGetWindowUserPointer(win));
            input->windowSizeCallback_(x, y);
        });
}
} // namespace src::windowmanagement