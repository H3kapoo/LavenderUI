#pragma once

#include <cstdint>
#include <queue>
#include <unordered_set>

#include "src/Node/UIBase.hpp"
#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Node/Helpers/UIState.hpp"
#include "src/Core/Binders/WindowBinder.hpp"
#include "src/Utils/Logger.hpp"

namespace lav::node
{
/**
    @brief
    Class representing a top level UI element aka Window. This acts as the root of the whole
    UI responsible for updating child elements.
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

    auto run() -> bool;
    auto quit() -> void;

    auto setTitle(std::string title, const bool updateInteralText = true) -> void;
    auto getTitle() -> std::string;
    auto getWindow() -> core::WindowHandle;
    auto getDeltaTime() -> double;
    auto isMainWindow() -> bool;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UIWindow);

private:
    using RawEventCallback = std::function<void()>;

    auto setupInputCallbacks() -> void;
    auto initializeDefaultCursor() -> void;
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

    auto onRender(const glm::mat4& projection) -> void override;
    auto onLayout() -> void override;
    auto onEvent(UIStatePtr& state) -> void override;

    auto shouldElementBeRendered(const UIBasePtr& node) -> bool;
    auto shouldLayoutBeComputedForElement(const UIBasePtr& node) -> bool;
    auto setupScissorAreaForElement(const UIBasePtr& node, const glm::mat4& projection) -> void;
    auto setupStaticViewBoundsForElement(const UIBasePtr& element) -> void;
    auto calculateDynamicViewBoundsForChildElements(const UIBasePtr& element) -> void;

private:
    core::WindowBinder::InputCallbacks cbs_;
    core::WindowHandle window_;
    glm::mat4 projection_;
    std::string title_;
    std::queue<UIBasePtr> processingQueue_;
    UIStatePtr uiState_{utils::make<UIState>()};
    double startTime_{0};
    double deltaTime_{0};
    bool shouldManuallyQuit{false};
    bool isMainWindow_{false};

    // Raw events
    std::vector<RawEventCallback> pendingRawEventCallbacks_;
    std::unordered_set<uint32_t> pendingRawEventIds_;
    bool isElementRemovedViaEvent_{false};

    static int32_t MAX_LAYERS;
    static bool isFirstWindow_;
};
using UIWindowPtr = std::shared_ptr<UIWindow>;
using UIWindowWPtr = std::weak_ptr<UIWindow>;
} // namespace lav::node