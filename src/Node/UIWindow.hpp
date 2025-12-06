#pragma once

#include <cstdint>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>

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
using RawEventCallback = std::function<void()>;

public:
    UIWindow(const std::string& title, const glm::ivec2& size);
    virtual ~UIWindow();
    UIWindow(const UIWindow&) = delete;
    UIWindow(UIWindow&&) = delete;
    auto operator=(UIWindow&&) -> UIWindow& = delete;
    auto operator=(const UIWindow&) -> UIWindow& = delete;

    auto run() -> bool;
    auto quit() -> void;

    auto setTitle(std::string title, const bool updateInteralText = true) -> void;
    auto getTitle() -> std::string;
    auto getWindow() -> core::WindowHandle;
    auto isMainWindow() -> bool;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UIWindow);

private:
    auto setupInputCallbacks() -> void;
    auto initializeDefaultCursors() -> void;
    auto updateWindowSizeAndProjection(const glm::ivec2 newSize) -> void;
    auto insertUniquePendingRawEvent(const core::IEvent& e, const RawEventCallback& cb) -> void;
    auto clearAllUniquePendingRawEvents() -> void;
    auto resolvePendingRawEvents() -> void;
    auto emitEventTo(const core::IEvent& event, const std::optional<uint32_t> nodeId) -> void;
    auto scanForHoveredNode() -> void;

    auto mouseMoveSolver(const int32_t newX, const int32_t newY) -> void;
    auto mouseButtonSolver(const uint32_t btn, const uint32_t action) -> void;
    auto mouseScrollSolver(const uint32_t xOffset, const uint32_t yOffset) -> void;
    auto windowResizeSolver(const uint32_t newX, const uint32_t newY) -> void;
    auto windowMouseEnterSolver(const bool entered) -> void;
    auto keyButtonSolver(const uint32_t key, const uint32_t scancode, const uint32_t action,
        const uint32_t mods) -> void;

    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(UIStatePtr& state) -> void override;

    auto areRenderPreconditionsSatisfied(const UIBasePtr& node) -> bool;
    auto areLayoutPreconditionsSatisfied(const UIBasePtr& node) -> bool;
    auto preRenderSetup(const UIBasePtr& node, const glm::mat4& projection) -> void;
    auto preLayoutSetup(const UIBasePtr& node) -> void;
    auto postRenderActions(const UIBasePtr& node) -> void;
    auto postLayoutActions(const UIBasePtr& node) -> void;

private:
    core::WindowBinder::InputCallbacks cbs_;
    core::WindowHandle window_;
    glm::mat4 projection_;
    std::string title_;
    std::queue<UIBasePtr> processingQueue_;
    UIStatePtr uiState_{utils::make<UIState>()};
    bool forcedQuit_{false};
    bool isMainWindow_{false};

    // Raw events
    std::vector<RawEventCallback> pendingRawEventCallbacks_;
    std::unordered_set<uint32_t> pendingRawEventIds_;
    std::vector<UIBasePtr> removedNodesDuringEvent_;
    bool isElementRemovedViaEvent_{false};

    static int32_t MAX_LAYERS;
    static bool isFirstWindow_;
};
using UIWindowPtr = std::shared_ptr<UIWindow>;
using UIWindowWPtr = std::weak_ptr<UIWindow>;
} // namespace lav::node