#include <LavenderUI/App.hpp>

#include <algorithm>
#include <memory>

#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/Binders/WindowBinder.hpp>
#include <LavenderUI/Core/ViewParser/LavParser.hpp>
#include <LavenderUI/Node/UIBase.hpp>

namespace lav
{
App::App()
    : log_(__func__)
    , keepRunning_(true)
    , shouldUpdateTitle_(false)
    , showFps_(false)
{
    // log_.warn("capacity {}", windows_.capacity());
    // windows_.reserve(6);
}

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
    node::UIWindowPtr window = utils::make<node::UIWindow>(title, size);
    return windows_.emplace_back(window);
}

auto App::findWindow(const uint32_t windowId) -> node::UIWindowWPtr
{
    const auto it = std::ranges::find_if(windows_,
        [windowId](const uint32_t id) { return id == windowId; },
        [](const node::UIWindowPtr& w) { return w->getId(); });

    return it != windows_.end() ? *it : std::weak_ptr<node::UIWindow>{};
}

auto App::run() -> void
{
    shouldUpdateTitle_ = true;
    while (keepRunning_)
    {
        /*
            Iterate and update the initial amount of windows.
            If a window get inserted while iterating, we will process it the next frame.
            If a window gets removed, we save it's ID and after all initial windows have been processed
                the saved windows with those IDs will be removed.
            This guarantees there will be no UB due to add/removal of windows.
        */
        const uint32_t initialWindowsCount = windows_.size();
        for (uint32_t i = 0; i < initialWindowsCount; ++i)
        {
            const uint32_t id = windows_[i]->getId();
            if (runPerWindow(windows_[i])) { toRemoveWindows_.insert(id); }
        }

        if (!toRemoveWindows_.empty())
        {
            std::erase_if(windows_, [this](const auto& w)
            {
                return toRemoveWindows_.contains(w->getId());
            });

            toRemoveWindows_.clear();
        }

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

auto App::runPerWindow(const node::UIWindowPtr window) -> bool
{
    const bool shouldFrameBeRemoved = window->run();
    if (shouldFrameBeRemoved && window->isMainWindow())
    {
        keepRunning_ = false;
    }

    if (showFps_ && shouldUpdateTitle_)
    {
        // TODO: FPS counting is still all over the place in high FPS range when multiple windows are present.
        const auto fps = std::to_string(1.0f / window->getDeltaTime());
        const auto title = window->getTitle();
        window->setTitle(title + " | " + fps, false);
    }

    return shouldFrameBeRemoved;
}
} // namespace lav