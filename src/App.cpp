#include "App.hpp"
#include "src/Utils/Logger.hpp"
#include "vendor/glfw/include/GLFW/glfw3.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace src
{
using namespace windowmanagement;

App::App()
    : log_("App")
{}

App::~App()
{
    frames_.clear();
    NativeWindow::terminate();
}

auto App::init() -> bool
{
    return NativeWindow::init();
}

auto App::createFrame(const std::string& title, const glm::ivec2 size) -> uielements::UIFrameWPtr
{
    uielements::UIFramePtr frame = std::make_shared<uielements::UIFrame>(title, size);
    return frames_.emplace_back(frame);
}

auto App::findFrame(const uint64_t frameId) -> uielements::UIFrameWPtr
{
    const auto it = std::ranges::find_if(frames_,
        [frameId](const uint64_t id) { return id == frameId; },
        [](const uielements::UIFramePtr& f) { return f->getId(); });

    return it != frames_.end() ? *it : std::weak_ptr<uielements::UIFrame>{};
}

auto App::run() -> void
{
    double storedTime{glfwGetTime()};
    bool updateTitle{false};
    while (runCondition_)
    {
        std::erase_if(frames_, std::bind(&App::runPerWindow, this, std::placeholders::_1, updateTitle));

        if (frames_.empty()) { break; }

        updateTitle = false;
        if (const double nowTime = glfwGetTime(); nowTime - storedTime >= 1)
        {
            updateTitle = true;
            storedTime = nowTime;
        }

        NativeWindow::nextEvent();
    }
}

auto App::get() -> App&
{
    static App instance;
    return instance;
}

auto App::setWaitEvents(const bool waitEvents) -> void
{
    NativeWindow::setWaitEvents(waitEvents);
}

auto App::runPerWindow(const uielements::UIFramePtr& frame, const bool updateTitle) -> bool
{
    NativeWindow& window = frame->getWindow();
    const bool shouldFrameBeRemoved = frame->run();
    if (shouldFrameBeRemoved && frame->isMainFrame())
    {
        runCondition_ = false;
    }

    if (updateTitle)
    {
        window.updateTitle(std::to_string(1.0f / window.getDeltaTime()));
    }

    return shouldFrameBeRemoved;
}
} // namespace src