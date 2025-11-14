#pragma once

#include <string>
#include <functional>

#ifdef __linux__
#include <GL/glx.h>
#endif

#define LAV_USE_GLFW_WINDOWING

#ifdef LAV_USE_GLFW_WINDOWING
#define GLFW_EXPOSE_NATIVE_X11
#include "vendor/glfw/include/GLFW/glfw3.h"
#include "vendor/glfw/include/GLFW/glfw3native.h"
#endif

#include "src/Utils/Logger.hpp"
#include "vendor/glm/glm.hpp"

    namespace lav
{
enum Cursor : uint32_t;
}

namespace lav::core
{
#ifdef LAV_USE_GLFW_WINDOWING
using WindowHandle = GLFWwindow*;
using WindowCursor = GLFWcursor*;
#endif

using KeyCallback = std::function<void(int32_t key, int32_t scanCode, int32_t action, int32_t mods)>;
using CharacterCallback = std::function<void(uint32_t codepoint)>;
using MouseButtonCallback = std::function<void(uint8_t btn, uint8_t action)>;
using MouseMoveCallback = std::function<void(int32_t x, int32_t y)>;
using MouseScrollCallback = std::function<void(int8_t xOffset, int8_t yOffset)>;
using WindowSizeCallback = std::function<void(uint32_t x, uint32_t y)>;
using WindowMouseEnterCallback = std::function<void(bool entered)>;
using WindowFileDropCallback = std::function<void(int32_t count, const char** paths)>;

class WindowBinder
{
public:
    struct InputCallbacks
    {
        KeyCallback keyCallback{[](auto, auto, auto, auto){}};
        CharacterCallback characterCallback{[](auto){}};
        MouseMoveCallback mouseMoveCallback{[](auto, auto){}};
        MouseButtonCallback mouseBtnCallback{[](auto, auto){}};
        MouseScrollCallback mouseScrollCallback{[](auto, auto){}};
        WindowSizeCallback windowSizeCallback{[](auto, auto){}};
        WindowMouseEnterCallback windowMouseEntered{[](auto){}};
        WindowFileDropCallback windowFileDrop{[](auto, auto){}};
    };

public:
    static auto get() -> WindowBinder&;

    auto init() -> bool;
    auto terminate() -> void;
    auto createWindow(const std::string& title, const glm::ivec2 size) -> WindowHandle;
    auto makeContextCurrent(WindowHandle handle) -> void;
    auto enableVSync(const bool) -> void;
    auto maskEvents(WindowHandle handle) -> void;
    auto setCursor(WindowHandle handle, WindowCursor cursor) -> void;
    auto setStandardCursor(WindowHandle handle, lav::Cursor cursor) -> void;
    auto destroyCursor(WindowCursor cursor) -> void;
    auto swapBuffers(WindowHandle handle) -> void;
    auto shouldWindowClose(WindowHandle handle) -> bool;
    auto close(WindowHandle handle) -> void;
    auto setTitle(WindowHandle handle, const std::string& title) -> void;
    auto setPollWaitForEvents(const bool wait) -> void;
    auto pollEvents() -> void;
    auto getTime() -> double;
    auto destroyWindow(WindowHandle handle) -> void;

    auto setInputCallbacks(WindowHandle handle, const InputCallbacks& cbs) -> void;

private:
    WindowBinder() = default;
    WindowBinder(const WindowBinder&) = delete;
    WindowBinder(WindowBinder&&) = delete;
    auto operator=(const WindowBinder&) = delete;
    auto operator=(WindowBinder&&) = delete;

    auto setUserPointer(WindowHandle handle, void* data) -> void;

private:
    utils::Logger log_{"WindowBinder"};
    bool pollingMethodIsWait_{true};
    std::unordered_map<lav::Cursor, GLFWcursor*> cursors_;

    WindowHandle initWindowHandle_{nullptr};

#ifdef __linux__
    /*
        In order for all windows to share a single context, and thus the same resources, we need
        to go native, beyond normal handling. All resources will be shared with the init
        window created at the beggining of the app.
    */
    Display* initDisplay_{nullptr};
    GLXContext initContext_;
#endif
};
} // namespace lav::core

namespace lav
{
enum Action : uint8_t
{
    RELEASE = GLFW_RELEASE,
    PRESS = GLFW_PRESS,
    REPEAT = GLFW_REPEAT
};

enum Mouse : uint8_t
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

enum Cursor : uint32_t
{
    ARROW = GLFW_ARROW_CURSOR,
    IBEAM = GLFW_IBEAM_CURSOR,
    CROSSHAIR = GLFW_CROSSHAIR_CURSOR,
    HAND = GLFW_HAND_CURSOR,
    HRESIZE = GLFW_HRESIZE_CURSOR,
    VRESIZE = GLFW_VRESIZE_CURSOR,
    ALLRESIZE = GLFW_RESIZE_ALL_CURSOR,
    NOT_ALLOWED = GLFW_NOT_ALLOWED_CURSOR
};

enum Key : uint32_t
{
    ESC = GLFW_KEY_ESCAPE,
    C = GLFW_KEY_C,
    P = GLFW_KEY_P
};
}