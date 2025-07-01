#include "Input.hpp"

#include "vendor/glfw/include/GLFW/glfw3.h"

namespace src::windowmanagement
{
#define SETUP_CALLBACK(cbName, cbType_, cbVarName_)\
auto Input::cbName(const cbType_& callback) -> void\
{\
    cbVarName_ = callback;\
}\

SETUP_CALLBACK(setKeyCallback, KeyCallback, keyCallback_);
SETUP_CALLBACK(setMouseMoveCallback, MouseMoveCallback, mouseMoveCallback_);
SETUP_CALLBACK(setMouseBtnCallback, MouseButtonCallback, mouseBtnCallback_);
SETUP_CALLBACK(setMouseScrollCallback, MouseScrollCallback, mouseScrollCallback_);
SETUP_CALLBACK(setWindowSizeCallback, WindowSizeCallback, windowSizeCallback_);

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
    
    glfwSetCursorPosCallback(windowHandle,
        [](GLFWwindow* win, double x, double y)
        {
            const Input* input = static_cast<Input*>(glfwGetWindowUserPointer(win));
            input->mouseMoveCallback_(x, y);
        });
    
    glfwSetMouseButtonCallback(windowHandle,
        [](GLFWwindow* win, int32_t btn, int32_t action, int32_t)
        {
            const Input* input = static_cast<Input*>(glfwGetWindowUserPointer(win));
            input->mouseBtnCallback_(btn, action);
        });

    glfwSetScrollCallback(windowHandle,
        [](GLFWwindow* win, double xOffset, double yOffset)
        {
            const Input* input = static_cast<Input*>(glfwGetWindowUserPointer(win));
            input->mouseScrollCallback_(xOffset, yOffset);
        });
}

#undef SETUP_CALLBACK
} // namespace src::windowmanagement