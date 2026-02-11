#pragma once

#include <cstdint>
#include <queue>
#include <unordered_set>

#include <LavenderUI/Node/UIBase.hpp>
#include <LavenderUI/Core/EventHandler/IEvent.hpp>
#include <LavenderUI/Core/State/UIState.hpp>
#include <LavenderUI/Core/Binders/WindowBinder.hpp>

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
    auto setSize(const glm::ivec2 size) -> void;
    auto setPosition(const glm::ivec2 topLeftPos) -> void;
    auto setFullScreen(const bool fullscreen = true) -> void;
    auto getTitle() -> std::string;
    auto getWindow() -> core::WindowHandle;
    auto getDeltaTime() -> double;
    auto getSize() const -> glm::ivec2;
    auto getPosition() const -> glm::ivec2;
    auto getWindowMonitorSize() const -> glm::ivec2;
    auto isFullScreen() -> bool;
    auto isMainWindow() -> bool;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UIWindow);

private:
    using RawEventCallback = std::function<void()>;

    auto resolvePendingRawEvents() -> void;
    auto resolveLayoutTask() -> bool;
    auto resolveRenderTask() -> void;
    auto setupInputCallbacks() -> void;
    auto initializeDefaultCursor() -> void;
    auto updateWindowSizeAndProjection(const glm::ivec2 newSize) -> void;
    auto insertUniquePendingRawEvent(const core::IEvent& e, const RawEventCallback& cb) -> void;
    auto clearAllUniquePendingRawEvents() -> void;
    auto emitEventTo(const core::IEvent& event, const std::optional<uint32_t> nodeId) -> void;
    auto scanForHoveredNode() -> void;

    auto mouseMoveSolver(const int32_t newX, const int32_t newY) -> void;
    auto mouseButtonSolver(const uint32_t btn, const uint32_t action) -> void;
    auto mouseScrollSolver(const uint32_t xOffset, const uint32_t yOffset) -> void;
    auto windowResizeSolver(const uint32_t newX, const uint32_t newY) -> void;
    auto windowMouseEnterSolver(const bool entered) -> void;
    auto windowFocusSolver(const bool focused) -> void;
    auto keyButtonSolver(const uint32_t key, const uint32_t scancode, const uint32_t action,
        const uint32_t mods) -> void;
    auto characterSolver(const uint32_t codepoint) -> void;

    auto onRender(const glm::mat4& projection) -> void override;
    auto onLayout() -> void override;
    auto onEvent(core::UIStatePtr& state) -> void override;

    auto shouldElementBeRendered(const UIBasePtr& node) -> bool;
    auto shouldLayoutBeComputedForElement(const UIBasePtr& node) -> bool;
    auto setupScissorAreaForElement(const UIBasePtr& node, const glm::mat4& projection) -> void;
    auto setupStaticViewBoundsForElement(const UIBasePtr& element) -> void;
    auto calculateDynamicViewBoundsForChildElements(const UIBasePtr& element) -> void;

private:
    core::WindowBinder::InputCallbacks cbs_;
    std::queue<UIBasePtr> processingQueue_;
    std::vector<RawEventCallback> pendingRawEventCallbacks_;
    std::unordered_set<uint32_t> pendingRawEventIds_;
    core::WindowHandle window_;
    glm::mat4 projection_;
    std::string title_;
    core::UIStatePtr uiState_;
    double startTime_;
    double deltaTime_;
    bool shouldManuallyQuit_;
    bool isMainWindow_;

    static bool isFirstWindow_;
    const static int32_t MAX_UNIQUE_EVENTS_;
    const static int32_t DROPDOWN_ROOT_HOLDER_SEPARATION_INDEX;
    const static int32_t MAX_LAYERS_;
};
using UIWindowPtr = std::shared_ptr<UIWindow>;
using UIWindowWPtr = std::weak_ptr<UIWindow>;
} // namespace lav::node