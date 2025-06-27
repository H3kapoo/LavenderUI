#pragma once

#include <functional>

#include "vendor/glfw/include/GLFW/glfw3.h"

namespace src::windowmanagement
{
using KeyCallback = std::function<void(int32_t key, int32_t scanCode, int32_t action, int32_t mods)>;
using MouseButtonCallback = std::function<void(uint8_t btn, uint8_t action)>;
using MouseMoveCallback = std::function<void(int32_t x, int32_t y)>;
using WindowSizeCallback = std::function<void(uint32_t x, uint32_t y)>;

class NativeWindow;
class Input
{
public:
    Input() = default;
    ~Input() = default;
    Input(const Input&) = delete;
    Input(Input&&) = delete;
    Input& operator=(const Input&) = delete;
    Input& operator=(Input&&) = delete;

    auto setKeyCallback(const KeyCallback& callback) -> void;
    auto setMouseMoveCallback(const MouseMoveCallback& callback) -> void;
    auto setMouseBtnCallback(const MouseButtonCallback& callback) -> void;
    auto setWindowSizeCallback(const WindowSizeCallback& callback) -> void;

private:
    friend NativeWindow;
    auto bindWindow(GLFWwindow* windowHandle) -> void;

private:
    KeyCallback keyCallback_{[](auto, auto, auto, auto){}};
    MouseMoveCallback mouseMoveCallback_{[](auto, auto){}};
    MouseButtonCallback mouseBtnCallback_{[](auto, auto){}};
    WindowSizeCallback windowSizeCallback_{[](auto, auto){}};
};
} // namespace src::windowmanagement