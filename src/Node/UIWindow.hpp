#pragma once

#include <queue>
#include <unordered_map>

#include "src/Node/UIBase.hpp"
#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Node/Helpers/UIState.hpp"
#include "src/Core/Binders/WindowBinder.hpp"

namespace lav::node
{

/**
    @brief Top level GUI container wrapping an OS window. Manages dispatch of layout calculations,
        rendering, layout and events handling for all children GUI elements.

    @note Each UIWindow has it's own global UIWindowState handle.
*/
class UIWindow : public UIBase
{
public:
    UIWindow(const std::string& title, const glm::ivec2& size);
    virtual ~UIWindow();
    UIWindow(const UIWindow&) = delete;
    UIWindow(UIWindow&&) = delete;
    auto operator=(UIWindow&&) -> UIWindow& = delete;
    auto operator=(const UIWindow&) -> UIWindow& = delete;

    /**
        @brief Run the window's logic one time.

        @note This is not the same as the App::run() method as UIWindows do not have their
            standalone game loop.
        @note This is public in order to give the user the freedom to make his own loop.

        @return True if window shall close. False otherwise.
    */
    auto run() -> bool;

    /**
        @brief Quit this window or quit the whole program if this window is the primary one.
    */
    auto quit() -> void;

    auto setTitle(std::string title, const bool updateInteralText = true) -> void;
    auto getDeltaTime() -> double;
    auto getTitle() -> std::string;
    auto getWindow() -> core::WindowHandle;
    auto isMainWindow() -> bool;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UIWindow);
    BASE_ALLOW_ADD_REMOVE;

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(UIStatePtr& state) -> void override;
    auto windowResizeHook(const uint32_t newX, const uint32_t newY) -> void;
    auto windowMouseEnterHook(const bool entered) -> void;
    auto keyHook(const uint32_t key, const uint32_t scancode, const uint32_t action,
        const uint32_t mods) -> void;
    auto mouseMoveHook(const int32_t newX, const int32_t newY) -> void;
    auto mouseButtonHook(const uint32_t btn, const uint32_t action) -> void;
    auto mouseScrollHook(const uint32_t xOffset, const uint32_t yOffset) -> void;
    auto propagateEventTo(const core::IEvent& event, const std::optional<uint32_t> nodeId) -> void;
    auto updateWindowSizeAndProjection(const glm::ivec2 newSize) -> void;
    auto initializeDefaultCursors() -> void;
    auto areRenderPreconditionsSatisfied(const UIBasePtr& node) -> bool;
    auto areLayoutPreconditionsSatisfied(const UIBasePtr& node) -> bool;
    auto preRenderSetup(const UIBasePtr& node, const glm::mat4& projection) -> void;
    auto preLayoutSetup(const UIBasePtr& node) -> void;
    auto propagateHoverScanEvent() -> void;
    auto postRenderActions(const UIBasePtr& node) -> void;
    auto postLayoutActions(const UIBasePtr& node) -> void;

private:
    core::WindowHandle window_;
    std::string title_;
    glm::mat4 projection_;
    std::queue<UIBasePtr> processingQueue_;
    UIStatePtr uiState_{utils::make<UIState>()};
    bool forcedQuit_{false};
    bool isMainWindow_{false};
    core::WindowBinder::InputCallbacks cbs_;

    static int32_t MAX_LAYERS;
    static bool isFirstWindow_;
};
using UIWindowPtr = std::shared_ptr<UIWindow>;
using UIWindowWPtr = std::weak_ptr<UIWindow>;
} // namespace lav::node