#pragma once

#include <memory>
#include <string>

#define GLFW_EXPOSE_NATIVE_X11
#include "vendor/glew/include/GL/glew.h"
#include "vendor/glfw/include/GLFW/glfw3.h"
#include "vendor/glfw/include/GLFW/glfw3native.h"
#include "src/Core/WindowHandler/Input.hpp"
#include "vendor/glm/glm.hpp"

#include "src/Utils/Logger.hpp"

#ifdef __linux__
#include <GL/glx.h>
#endif

namespace lav::core
{
/**
    @brief Class responsible for initializing and wrapping the native OS window provided by GLFW.
        Furthermore anything related to window management from GLEW side is handled here as well.
*/
class NativeWindow
{
public:
    NativeWindow(const std::string& title, const glm::ivec2 size);
    ~NativeWindow();
    NativeWindow(const NativeWindow&) = delete;
    NativeWindow(NativeWindow&&) = delete;
    auto operator=(const NativeWindow&) -> NativeWindow& = delete;
    auto operator=(NativeWindow&&) -> NativeWindow& = delete;

    /**
        @brief Tells the window API to close the window.
    */
    auto close() const -> void;

    /**
        @brief Makes the openGL context be the current one.
    */
    auto makeContextCurrent() -> void;

    /**
        @brief Checks if the window shall be closed.

        @return True if window shall close. False otherwise.
    */
    auto shouldWindowClose() const -> bool;

    /**
        @brief Swap back and front buffers.
    */
    auto swapBuffers() -> void;

    /**
        @brief Initialize GLFW and GLEW libs.

        @return True on success. Flase on failure.
    */
    static auto init() -> bool;

    /**
        @brief Terminate GLFW allocated resources.
    */
    static auto terminate() -> void;

    /**
        @brief Changes vsync of the windows.

        @note Vsync change will affect all the windows at least on Linux.

        @param enable Enable of not vsync
    */
    static auto enableVsync(const bool enable = true) -> void;

    /**
        @brief Make the window wait or not for events before continuing the game loop.

        @param waitEvents Wait for the next event or not
    */
    static auto setWaitEvents(const bool waitEvents = true) -> void;

    /**
        @brief Just poll the currently in the queue events or wait for events.
    */
    static auto pollEvents() -> void;

    auto setTitle(std::string title, const bool updateInteralText) -> void;

    auto getGlfwHandle() const -> GLFWwindow*;
    auto getTitle() const -> std::string;
    auto getSize() const -> glm::ivec2;
    auto getId() const -> uint64_t;
    auto getDeltaTime() -> double;
    auto getInput() -> Input&;

    static auto setClipboardData(const std::string& data) -> void;
    static auto enableDepthTest(const bool enable = true) -> void;
    static auto enableScissorsTest(const bool enable = true) -> void;
    static auto enableAlphaBlending(const bool enable = true) -> void;
    static auto updateScissors(const glm::vec4& area) -> void;
    static auto clearColor(const glm::vec4& color) -> void;
    static auto clearBuffers(uint32_t bufferBits) -> void;
    static auto clearAllBuffers() -> void;
    static auto updateViewport(const glm::ivec2 size) -> void;
    static auto getInitGlfwHandle() -> GLFWwindow*;
    static auto getTime() -> double;
    static auto getClipboardData() -> std::string;

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
    window created at the beggining of the app. */
#ifdef __linux__
    static Display* initDisplay_;
    static GLXContext initContext_;
#endif
};
using WindowPtr = std::shared_ptr<NativeWindow>;
using WindowWPtr = std::weak_ptr<NativeWindow>;
} // namespace lav::core