#include "App.hpp"

#include <algorithm>

#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Core/Binders/WindowBinder.hpp"
#include "src/Core/LavParser/LavParser.hpp"
#include "src/Node/UIBase.hpp"

namespace lav
{
App::~App()
{
    windows_.clear();
    core::WindowBinder::get().terminate();
}

auto App::init() -> bool
{
    return core::WindowBinder::get().init() && core::GPUBinder::get().init();
}

auto App::loadLavView(const std::filesystem::path& viewPath) -> node::UIWindowWPtr
{
    node::UIBasePtrVec windowElements = core::LavParser::get().parseFromFile(viewPath);
    auto window = utils::as<node::UIWindow>(windowElements[0]);
    windows_.emplace_back(window);
    return window;
}

auto App::createWindow(const std::string& title, const glm::ivec2 size) -> node::UIWindowWPtr
{
    node::UIWindowPtr frame = utils::make<node::UIWindow>(title, size);
    return windows_.emplace_back(frame);
}

auto App::findWindow(const uint64_t windowId) -> node::UIWindowWPtr
{
    const auto it = std::ranges::find_if(windows_,
        [windowId](const uint64_t id) { return id == windowId; },
        [](const node::UIWindowPtr& w) { return w->getId(); });

    return it != windows_.end() ? *it : std::weak_ptr<node::UIWindow>{};
}

auto App::run() -> void
{
    shouldUpdateTitle_ = true;
    while (keepRunning_)
    {
        /* TODO: The FPS counter is broken whenever we have multiple windows. Not sure
            how nicely it will play out with future animations. */
        const double startTime{core::WindowBinder::get().getTime()};
        std::erase_if(windows_, [this](const auto& w) { return runPerWindow(w); });
        const double now = core::WindowBinder::get().getTime();
        deltaTime_ = 1.0f / (now - startTime);

        core::WindowBinder::get().pollEvents();

        if (windows_.empty()) { break; }
    }
}

auto App::get() -> App&
{
    static App instance;
    return instance;
}

auto App::setWaitEvents(const bool waitEvents) -> void
{
    core::WindowBinder::get().setPollWaitForEvents(waitEvents);
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
        const auto fps = std::to_string(deltaTime_);
        const auto title = window->getTitle();
        window->setTitle(title + " | " + fps, false);
    }

    return shouldFrameBeRemoved;
}
} // namespace lav