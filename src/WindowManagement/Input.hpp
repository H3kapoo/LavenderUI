#pragma once

#include <functional>

#include "vendor/glfw/include/GLFW/glfw3.h"

namespace src::windowmanagement
{
using KeyCallback = std::function<void(int32_t key, int32_t scanCode, int32_t action, int32_t mods)>;
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
    auto setWindowSizeCallback(const WindowSizeCallback& callback) -> void;

private:
    friend NativeWindow;
    auto bindWindow(GLFWwindow* windowHandle) -> void;

private:
    KeyCallback keyCallback_{[](auto, auto, auto, auto){}};
    WindowSizeCallback windowSizeCallback_{[](auto, auto){}};
};
} // namespace src::windowmanagement