#pragma once

#include <memory>
#include <string>

#define GLFW_EXPOSE_NATIVE_X11
#include "vendor/glew/include/GL/glew.h"
#include "vendor/glfw/include/GLFW/glfw3.h"
#include "vendor/glfw/include/GLFW/glfw3native.h"
#include "src/WindowManagement/Input.hpp"
#include "vendor/glm/glm.hpp"
#include "src/Utils/Logger.hpp"

#ifdef __linux__
#include <GL/glx.h>
#endif

namespace src::windowmanagement
{
class NativeWindow
{
public:
    enum Property
    {
        DEPTH_TEST = GL_DEPTH_TEST,
        SCISSOR_TEST = GL_SCISSOR_TEST,
        ALPHA_BLENDING = GL_BLEND,
    };

    enum BlendingFunc
    {
        CLASSIC = GL_ONE_MINUS_SRC_ALPHA
    };

public:
    NativeWindow(const std::string& title, const glm::ivec2 size);
    ~NativeWindow();
    NativeWindow(const NativeWindow&) = delete;
    NativeWindow& operator=(const NativeWindow&) = delete;
    NativeWindow(NativeWindow&&) = delete;
    NativeWindow& operator=(NativeWindow&&) = delete;

    auto doCloseWindow() const -> void;
    auto makeContextCurrent() -> void;
    auto shouldWindowClose() const -> bool;
    auto swapBuffers() -> void;
    auto updateTitle(const std::string title) -> void;

    auto getGlfwHandle() const -> GLFWwindow*;
    auto getTitle() const -> std::string;
    auto getSize() const -> glm::ivec2;
    auto getId() const -> uint64_t;
    auto getDeltaTime() -> double;
    auto getInput() -> Input&;

public:
    static auto init() -> bool;
    static auto terminate() -> void;
    static auto setVsync(const bool capped = true) -> void;
    static auto setWaitEvents(const bool waitEvents = true) -> void;
    static auto nextEvent() -> void;
    static auto setProp(const Property prop, const bool enable = true) -> void;
    static auto setBlendFunc(const BlendingFunc fun) -> void;
    static auto updateScissors(glm::vec4 area) -> void;
    static auto clearColor(glm::vec4 color) -> void;
    static auto clearBuffers(uint32_t bufferBits) -> void;
    static auto updateViewport(const glm::ivec2 size) -> void;
    static auto getInitGlfwHandle() -> GLFWwindow*;

private:
    auto maskEvents() -> void;

private:
    uint64_t windowId_;
    utils::Logger log_;
    Input input_;
    std::string title_;
    GLFWwindow* windowHandle_;
    double startTime_;
    double delta_;

private:
    static GLFWwindow* initWindowHandle_;
    static bool isWaitingEvents_;

/* In order for all windows to share a single context, and thus the same resources, we need
    to go native, beyong glfw's normal handling. All resources will be shared with the init
    window created at the beggining of the app.
*/
#ifdef __linux__
    static Display* initDisplay_;
    static GLXContext initContext_;
#endif
};
using WindowPtr = std::shared_ptr<NativeWindow>;
using WindowWPtr = std::weak_ptr<NativeWindow>;
} // namespace src::windowmanagement