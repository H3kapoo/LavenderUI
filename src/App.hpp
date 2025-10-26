#pragma once

#include <vector>

#include "src/Node/UIWindow.hpp"
#include "src/Utils/Logger.hpp"

namespace lav
{
/**
    @brief Main app management entry point responsible for the lifetime of the GUI's windows.

    @note Always ensure init() is called before everything else happens.
    @note Only one instance of this can exist at any given time.
    @note Windows can be created from any thread. The first window is always the main window and closing it
        will close all the other windows.
    @note It is not advised to have an active shared_ptr handle to the window frame in the same scope
            as the run() command due to reference counting keeping the window alive even if the exit event was issued.
            You as the caller don't own anything the callee created.
*/
class App
{
public:
    App(const App&) = delete;
    App(App&&) = delete;
    auto operator=(const App&) -> App& = delete;
    auto operator=(App&&) -> App& = delete;

    /**
        @brief Initialize application specific libs.

        @note On successfull return it is guaranteed that an opengl is bound.

        @return True on success. False otherwise.
    */
    auto init() -> bool;

    /**
        @brief Create a new window of specified parameters.

        @note Caller shall not OWN a handle to the created window!

        @param title Title of the window
        @param size x, y size of the window in pixels

        @return Weak reference to the newly created window.
    */
    auto createWindow(const std::string& title, const glm::ivec2 size) -> node::UIWindowWPtr;

    /**
        @brief Find a window of specified id.

        @note Caller shall not OWN a handle to the returned window!

        @param windowId Id of the searched for window

        @return Weak reference to the newly created window.
    */
    auto findWindow(const uint64_t windowId) -> node::UIWindowWPtr;

    /**
        @brief Starts the GUI loop.

        @note Blocks until the app's main window is closed.
    */
    auto run() -> void;

    /**
        @brief Choose if the app shall run the loop at full speed or only when an OS window event is spawed.

        @note Default behavior is to wait for events to come in and not go full blast as it is useless
            in most applications that don't need constant refresh of UI.

        @param waitEvents Should wait for events or not
    */
    auto setWaitEvents(const bool waitEvents = true) -> void;

    /**
        @brief Show FPS counter near the window's title.

        @param enable Enable or not the counter
    */
    auto enableTitleWithFPS(const bool enable = true) -> void;

    /**
        @brief Static singleton accessor.

        @return Instance of the App class.
    */
    static auto get() -> App&;

private:
    App();
    ~App();

    auto runPerWindow(const node::UIWindowPtr& frame) -> bool;

private:
    utils::Logger log_{"App"};
    std::vector<node::UIWindowPtr> windows_;
    bool keepRunning_{true};
    bool shouldUpdateTitle_{false};
    bool showFps_{false};
};
} // namespace lav