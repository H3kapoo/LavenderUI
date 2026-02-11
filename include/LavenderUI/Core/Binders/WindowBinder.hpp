#pragma once

#include <string>
#include <functional>

#define LAV_USE_GLFW_WINDOWING

#include <glew/include/GL/glew.h>

#if defined(__linux__)
    #include <GL/glx.h>
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    /* Note: Cannot include <window.h> due to defines clash-up. */
    struct HGLRC__;
    using HGLRC = HGLRC__*;
#endif

#include <glfw/include/GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <LavenderUI/Utils/Logger.hpp>

/* Fwd declare default cursors enum type. */
namespace lav { enum class Cursor : uint32_t; }

namespace lav::core
{
#ifdef LAV_USE_GLFW_WINDOWING
    using WindowHandle = GLFWwindow*;
    using WindowCursor = GLFWcursor*;
#endif

using KeyCallback = std::function<void(
    const int32_t key, const int32_t scanCode, const int32_t action, const int32_t mods)>;
using CharacterCallback = std::function<void(const uint32_t codepoint)>;
using MouseButtonCallback = std::function<void(const uint8_t btn, const uint8_t action)>;
using MouseMoveCallback = std::function<void(const int32_t x, const int32_t y)>;
using MouseScrollCallback = std::function<void(const int8_t xOffset, const int8_t yOffset)>;
using WindowSizeCallback = std::function<void(const uint32_t x, const uint32_t y)>;
using WindowMouseEnterCallback = std::function<void(const bool entered)>;
using WindowFileDropCallback = std::function<void(const int32_t count, const char** paths)>;
using WindowFocusCallback = std::function<void(const bool focused)>;

class WindowBinder
{
public:
    struct InputCallbacks
    {
        KeyCallback                 keyCallback{[](auto, auto, auto, auto){}};
        CharacterCallback           characterCallback{[](auto){}};
        MouseMoveCallback           mouseMoveCallback{[](auto, auto){}};
        MouseButtonCallback         mouseBtnCallback{[](auto, auto){}};
        MouseScrollCallback         mouseScrollCallback{[](auto, auto){}};
        WindowSizeCallback          windowSizeCallback{[](auto, auto){}};
        WindowMouseEnterCallback    windowMouseEntered{[](auto){}};
        WindowFileDropCallback      windowFileDrop{[](auto, auto){}};
        WindowFocusCallback         windowFocus{[](auto){}};
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
    auto getPosition(WindowHandle handle) const -> glm::ivec2;
    auto getMonitorSize(WindowHandle handle) const -> glm::ivec2;
    auto requestEmptyEvent() -> void;
    auto destroyWindow(WindowHandle handle) -> void;

    auto setInputCallbacks(WindowHandle handle, const InputCallbacks& cbs) -> void;
    auto setSize(WindowHandle handle, const glm::ivec2 size) -> void;
    auto setPosition(WindowHandle handle, const glm::ivec2 topLeftPos) -> void;
    auto setFullScreen(WindowHandle handle, const bool fullscreen) -> void;

private:
    WindowBinder();
    WindowBinder(const WindowBinder&) = delete;
    WindowBinder(WindowBinder&&) = delete;
    auto operator=(const WindowBinder&) = delete;
    auto operator=(WindowBinder&&) = delete;

    auto setUserPointer(WindowHandle handle, void* data) -> void;

private:
    utils::Logger log_;
    bool pollingMethodIsWait_;
    WindowHandle initWindowHandle_;
    std::unordered_map<lav::Cursor, GLFWcursor*> cursors_;

#if defined(__linux__)
    /*
        In order for all windows to share a single context, and thus the same resources, we need
        to go native, beyond normal handling. All resources will be shared with the init
        window created at the beggining of the app. This is especially needed to share VAOs.
    */
    Display* initDisplay_{nullptr};
    GLXContext initContext_;

#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    HGLRC initContext_;
#endif
};
} // namespace lav::core

namespace lav
{
enum class Action : uint8_t
{
    RELEASE  = GLFW_RELEASE,
    PRESS    = GLFW_PRESS,
    REPEAT   = GLFW_REPEAT
};

enum class Mouse : uint8_t
{
    LEFT    = GLFW_MOUSE_BUTTON_LEFT,
    RIGHT   = GLFW_MOUSE_BUTTON_RIGHT,
    MIDDLE  = GLFW_MOUSE_BUTTON_MIDDLE,
    AUX_1   = GLFW_MOUSE_BUTTON_4,
    AUX_2   = GLFW_MOUSE_BUTTON_5,
    AUX_3   = GLFW_MOUSE_BUTTON_6,
    AUX_4   = GLFW_MOUSE_BUTTON_7,
    AUX_5   = GLFW_MOUSE_BUTTON_8,
};

enum class Cursor : uint32_t
{
    ARROW       = GLFW_ARROW_CURSOR,
    IBEAM       = GLFW_IBEAM_CURSOR,
    CROSSHAIR   = GLFW_CROSSHAIR_CURSOR,
    HAND        = GLFW_HAND_CURSOR,
    HRESIZE     = GLFW_HRESIZE_CURSOR,
    VRESIZE     = GLFW_VRESIZE_CURSOR,
    ALLRESIZE   = GLFW_RESIZE_ALL_CURSOR,
    NOT_ALLOWED = GLFW_NOT_ALLOWED_CURSOR
};

static inline const uint32_t KeyLastId = GLFW_KEY_LAST;

enum class Key : uint32_t
{
    SPACE        = GLFW_KEY_SPACE,
    APOSTROPHE   = GLFW_KEY_APOSTROPHE,
    COMMA        = GLFW_KEY_COMMA,
    MINUS        = GLFW_KEY_MINUS,
    PERIOD       = GLFW_KEY_PERIOD,
    SLASH        = GLFW_KEY_SLASH,
    NUM0         = GLFW_KEY_0, 
    NUM1         = GLFW_KEY_1, 
    NUM2         = GLFW_KEY_2, 
    NUM3         = GLFW_KEY_3, 
    NUM4         = GLFW_KEY_4, 
    NUM5         = GLFW_KEY_5, 
    NUM6         = GLFW_KEY_6, 
    NUM7         = GLFW_KEY_7, 
    NUM8         = GLFW_KEY_8, 
    NUM9         = GLFW_KEY_9, 
    SEMICOLON    = GLFW_KEY_SEMICOLON,
    EQUAL        = GLFW_KEY_EQUAL,
    A            = GLFW_KEY_A, 
    B            = GLFW_KEY_B, 
    C            = GLFW_KEY_C, 
    D            = GLFW_KEY_D, 
    E            = GLFW_KEY_E, 
    F            = GLFW_KEY_F, 
    G            = GLFW_KEY_G, 
    H            = GLFW_KEY_H, 
    I            = GLFW_KEY_I, 
    J            = GLFW_KEY_J, 
    K            = GLFW_KEY_K, 
    L            = GLFW_KEY_L, 
    M            = GLFW_KEY_M, 
    N            = GLFW_KEY_N, 
    O            = GLFW_KEY_O, 
    P            = GLFW_KEY_P, 
    Q            = GLFW_KEY_Q, 
    R            = GLFW_KEY_R, 
    S            = GLFW_KEY_S, 
    T            = GLFW_KEY_T, 
    U            = GLFW_KEY_U, 
    V            = GLFW_KEY_V, 
    W            = GLFW_KEY_W, 
    X            = GLFW_KEY_X, 
    Y            = GLFW_KEY_Y, 
    Z            = GLFW_KEY_Z, 
    L_BRACKET    = GLFW_KEY_LEFT_BRACKET,
    BACK_SLASH   = GLFW_KEY_BACKSLASH,
    R_BRACKET    = GLFW_KEY_RIGHT_BRACKET,
    GRAVE_ACCENT = GLFW_KEY_GRAVE_ACCENT,

    ESC                 = GLFW_KEY_ESCAPE,
    ENTER               = GLFW_KEY_ENTER,
    TAB                 = GLFW_KEY_TAB,
    BACKSPACE           = GLFW_KEY_BACKSPACE,
    INSERT              = GLFW_KEY_INSERT,
    DELETE              = GLFW_KEY_DELETE,
    RIGHT               = GLFW_KEY_RIGHT,
    LEFT                = GLFW_KEY_LEFT,
    DOWN                = GLFW_KEY_DOWN,
    UP                  = GLFW_KEY_UP,
    PAGE_UP             = GLFW_KEY_PAGE_UP,
    PAGE_DOWN           = GLFW_KEY_PAGE_DOWN,
    HOME                = GLFW_KEY_HOME,
    END                 = GLFW_KEY_END,
    CAPS_LOCK           = GLFW_KEY_CAPS_LOCK,
    SCROLL_LOCK         = GLFW_KEY_SCROLL_LOCK,
    NUM_LOCK            = GLFW_KEY_NUM_LOCK,
    PRINT_SCR           = GLFW_KEY_PRINT_SCREEN,
    PAUSE               = GLFW_KEY_PAUSE,
    F1                  = GLFW_KEY_F1,
    F2                  = GLFW_KEY_F2,
    F3                  = GLFW_KEY_F3,
    F4                  = GLFW_KEY_F4,
    F5                  = GLFW_KEY_F5,
    F6                  = GLFW_KEY_F6,
    F7                  = GLFW_KEY_F7,
    F8                  = GLFW_KEY_F8,
    F9                  = GLFW_KEY_F9,
    F10                 = GLFW_KEY_F10,
    F11                 = GLFW_KEY_F11,
    F12                 = GLFW_KEY_F12,
    F13                 = GLFW_KEY_F13,
    F14                 = GLFW_KEY_F14,
    F15                 = GLFW_KEY_F15,
    F16                 = GLFW_KEY_F16,
    F17                 = GLFW_KEY_F17,
    F18                 = GLFW_KEY_F18,
    F19                 = GLFW_KEY_F19,
    F20                 = GLFW_KEY_F20,
    F21                 = GLFW_KEY_F21,
    F22                 = GLFW_KEY_F22,
    F23                 = GLFW_KEY_F23,
    F24                 = GLFW_KEY_F24,
    F25                 = GLFW_KEY_F25,
    KEYPAD0             = GLFW_KEY_KP_0,
    KEYPAD1             = GLFW_KEY_KP_1,
    KEYPAD2             = GLFW_KEY_KP_2,
    KEYPAD3             = GLFW_KEY_KP_3,
    KEYPAD4             = GLFW_KEY_KP_4,
    KEYPAD5             = GLFW_KEY_KP_5,
    KEYPAD6             = GLFW_KEY_KP_6,
    KEYPAD7             = GLFW_KEY_KP_7,
    KEYPAD8             = GLFW_KEY_KP_8,
    KEYPAD9             = GLFW_KEY_KP_9,
    KEYPAD_DECIMAL      = GLFW_KEY_KP_DECIMAL,
    KEYPAD_DIDIDE       = GLFW_KEY_KP_DIVIDE,
    KEYPAD_MULTIPLY     = GLFW_KEY_KP_MULTIPLY,
    KEYPAD_SUBSTRACT    = GLFW_KEY_KP_SUBTRACT,
    KEYPAD_ADD          = GLFW_KEY_KP_ADD,
    KEYPAD_ENTER        = GLFW_KEY_KP_ENTER,
    KEYPAD_EQUAL        = GLFW_KEY_KP_EQUAL,
    L_SHIFT             = GLFW_KEY_LEFT_SHIFT,
    L_CONTROL           = GLFW_KEY_LEFT_CONTROL,
    L_ALT               = GLFW_KEY_LEFT_ALT,
    L_SUPER             = GLFW_KEY_LEFT_SUPER,
    R_SHIFT             = GLFW_KEY_RIGHT_SHIFT,
    R_CONTROL           = GLFW_KEY_RIGHT_CONTROL,
    R_ALT               = GLFW_KEY_RIGHT_ALT,
    R_SUPER             = GLFW_KEY_RIGHT_SUPER,
    MENU                = GLFW_KEY_MENU,
};
}