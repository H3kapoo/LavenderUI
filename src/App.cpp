#include "App.hpp"

#include <algorithm>

namespace lav
{
App::App()
{}

App::~App()
{
    windows_.clear();
    core::NativeWindow::terminate();
}

auto App::init() -> bool
{
    return core::NativeWindow::init();
}

auto App::createWindow(const std::string& title, const glm::ivec2 size) -> node::UIWindowWPtr
{
    node::UIWindowPtr frame = std::make_shared<node::UIWindow>(title, size);
    return windows_.emplace_back(frame);
}

auto App::findWindow(const uint64_t windowId) -> node::UIWindowWPtr
{
    const auto it = std::ranges::find_if(windows_,
        [windowId](const uint64_t id) { return id == windowId; }, // pred
        [](const node::UIWindowPtr& w) { return w->getId(); }); // proj

    return it != windows_.end() ? *it : std::weak_ptr<node::UIWindow>{};
}

auto App::run() -> void
{
    static uint32_t ONE_SECOND = 1.0f;
    double startTime{core::NativeWindow::getTime()};
    while (keepRunning_)
    {
        if (const double nowTime = core::NativeWindow::getTime(); nowTime - startTime >= ONE_SECOND)
        {
            shouldUpdateTitle_ = true;
            startTime = nowTime;
        }

        std::erase_if(windows_, [this](const auto& w) { return runPerWindow(w); });

        shouldUpdateTitle_ = false;

        if (windows_.empty()) { break; }

        core::NativeWindow::pollEvents();
    }
}

auto App::get() -> App&
{
    static App instance;
    return instance;
}

auto App::setWaitEvents(const bool waitEvents) -> void
{
    core::NativeWindow::setWaitEvents(waitEvents);
}

auto App::enableTitleWithFPS(const bool enable) -> void { showFps_ = enable; }

auto App::runPerWindow(const node::UIWindowPtr& window) -> bool
{
    const bool shouldFrameBeRemoved = window->run();
    if (shouldFrameBeRemoved && window->isMainWindow())
    {
        keepRunning_ = false;
    }

    if (showFps_ && shouldUpdateTitle_)
    {
        const auto fps = std::to_string(1.0f / window->getDeltaTime());
        const auto title = window->getTitle();
        window->setTitle(title + " | " + fps, false);
    }

    return shouldFrameBeRemoved;
}
} // namespace lav