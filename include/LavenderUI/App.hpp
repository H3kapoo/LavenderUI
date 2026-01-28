#pragma once

#include <filesystem>
#include <vector>

#include "include/LavenderUI/Node/UIWindow.hpp"
#include "include/LavenderUI/Utils/Logger.hpp"

namespace lav
{
/**
    @brief
    Main app management entry point responsible for the lifetime of the GUI's windows.

    @note 1. Always ensure init() is called before anything else happens.
    @note 2. Windows can be created from any thread. The first window is always the main window and closing it
        will close all the other windows.
    @note 3. It is not advised to have an active shared_ptr handle to the window frame in the same scope
            as the run() command due to reference counting keeping the window alive even if the exit event was issued.
            You as the caller don't own anything the callee created aka you only get a weak reference to the window.
    @note 4. Upon calling run(), thread will block until main window is closed.
*/
class App
{
public:
    static auto get() -> App&;

    auto init() -> bool;
    auto run() -> void;
    auto loadLavView(const std::filesystem::path& viewPath) -> node::UIWindowWPtr;
    auto createWindow(const std::string& title, const glm::ivec2 size) -> node::UIWindowWPtr;
    auto findWindow(const uint32_t windowId) -> node::UIWindowWPtr;
    auto setWaitEvents(const bool waitEvents = true) -> void;
    auto enableTitleWithFPS(const bool enable = true) -> void;

private:
    App();
    ~App();
    App(const App&) = delete;
    App(App&&) = delete;
    auto operator=(const App&) -> App& = delete;
    auto operator=(App&&) -> App& = delete;

    auto runPerWindow(const node::UIWindowPtr frame) -> bool;

private:
    utils::Logger log_{"App"};
    std::vector<node::UIWindowPtr> windows_;
    std::unordered_set<uint32_t> toRemoveWindows_;
    bool keepRunning_;
    bool shouldUpdateTitle_;
    bool showFps_;
};
} // namespace lav
