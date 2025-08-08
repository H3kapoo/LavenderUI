#pragma once

#include <functional>

#include "vendor/glfw/include/GLFW/glfw3.h"

namespace src::windowmanagement
{
using KeyCallback = std::function<void(int32_t key, int32_t scanCode, int32_t action, int32_t mods)>;
using MouseButtonCallback = std::function<void(uint8_t btn, uint8_t action)>;
using MouseMoveCallback = std::function<void(int32_t x, int32_t y)>;
using MouseScrollCallback = std::function<void(int8_t xOffset, int8_t yOffset)>;
using WindowSizeCallback = std::function<void(uint32_t x, uint32_t y)>;
using WindowMouseEnterCallback = std::function<void(bool entered)>;

class NativeWindow;

/**
    @brief Top level GUI input provider from the bound window.

    @note You can hook into these callbacks and generate events yourself if you want.
*/
class Input
{
public:
    Input() = default;
    ~Input() = default;
    Input(const Input&) = delete;
    Input(Input&&) = delete;
    auto operator=(const Input&) -> Input& = delete;
    auto operator=(Input&&) -> Input& = delete;

    auto setKeyCallback(const KeyCallback& callback) -> void;
    auto setMouseMoveCallback(const MouseMoveCallback& callback) -> void;
    auto setMouseBtnCallback(const MouseButtonCallback& callback) -> void;
    auto setMouseScrollCallback(const MouseScrollCallback& callback) -> void;
    auto setWindowSizeCallback(const WindowSizeCallback& callback) -> void;
    auto setWindowMouseEnterCallback(const WindowMouseEnterCallback& callback) -> void;

private:
    /* Only meant to be used by NativeWindow */
    friend NativeWindow;
    auto bindWindow(GLFWwindow* windowHandle) -> void;

private:
    KeyCallback keyCallback_{[](auto, auto, auto, auto){}};
    MouseMoveCallback mouseMoveCallback_{[](auto, auto){}};
    MouseButtonCallback mouseBtnCallback_{[](auto, auto){}};
    MouseScrollCallback mouseScrollCallback_{[](auto, auto){}};
    WindowSizeCallback windowSizeCallback_{[](auto, auto){}};
    WindowMouseEnterCallback windowMouseEntered_{[](bool){}};

public:
    enum Action
    {
        RELEASE = GLFW_RELEASE,
        PRESS = GLFW_PRESS,
        REPEAT = GLFW_REPEAT
    };

    enum Mouse
    {
        LEFT = GLFW_MOUSE_BUTTON_LEFT,
        RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
        MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE,
        AUX_1 = GLFW_MOUSE_BUTTON_4,
        AUX_2 = GLFW_MOUSE_BUTTON_5,
        AUX_3 = GLFW_MOUSE_BUTTON_6,
        AUX_4 = GLFW_MOUSE_BUTTON_7,
        AUX_5 = GLFW_MOUSE_BUTTON_8,
    };

    enum Key
    {
        ESC = GLFW_KEY_ESCAPE,
        C = GLFW_KEY_C,
        P = GLFW_KEY_P
    };
};
} // namespace src::windowmanagement