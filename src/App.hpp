#pragma once

#include <vector>

#include "src/UIElements/UIFrame.hpp"
#include "src/Utils/Logger.hpp"

namespace src
{
class App
{
public:
    App(const App&) = delete;
    App(App&&) = delete;
    App& operator=(const App&) = delete;
    App& operator=(App&&) = delete;

    auto init() -> bool;
    auto createFrame(const std::string& title, const glm::ivec2 size) -> uielements::UIFrameWPtr;
    auto findFrame(const uint64_t frameId) -> uielements::UIFrameWPtr;
    auto run() -> void;

    auto setWaitEvents(const bool waitEvents = true) -> void;

public:
    static auto get() -> App&;

private:
    App();
    ~App();

    auto runPerWindow(const uielements::UIFramePtr& frame, const bool updateTitle) -> bool;

private:
    utils::Logger log_;
    std::vector<uielements::UIFramePtr> frames_;
    bool runCondition_{true};
};
} // namespace src