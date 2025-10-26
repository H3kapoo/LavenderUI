#include "UIWindow.hpp"

#include <algorithm>

#include "src/App.hpp"
#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LayoutHandler/BasicCalculator.hpp"
#include "src/Node/UIBase.hpp"
// #include "src/Uinodes/UIDropdown.hpp"
#include "src/Utils/Misc.hpp"
#include "src/Core/WindowHandler/Input.hpp"
#include "src/Core/WindowHandler/NativeWindow.hpp"
#include "vendor/glfw/include/GLFW/glfw3.h"
#include "vendor/glm/ext/matrix_clip_space.hpp"

namespace lav::node
{
UIWindow::UIWindow(const std::string& title, const glm::ivec2& size)
    : UIBase({"UIWindow", "elemVert.glsl", "elemFrag.glsl"})
    , window_(title, size)
    , isMainWindow_(isFirstWindow_)
{
    initializeDefaultCursors();

    updateWindowSizeAndProjection(window_.getSize());

    /* Setup hooks into events */
    window_.getInput()
        .setWindowSizeCallback(
            [this](uint32_t x, uint32_t y) { windowResizeHook(x, y); })
        .setWindowMouseEnterCallback(
            [this](bool entered) { windowMouseEnterHook(entered); })
        .setCharacterCallback(
            [this](uint32_t cp){ (void)cp; })
        .setKeyCallback(
            [this](uint32_t key, uint32_t sc, uint32_t action, uint32_t mods)
            { keyHook(key, sc, action, mods); })
        .setMouseMoveCallback(
            [this](int32_t x, int32_t y) { mouseMoveHook(x, y); })
        .setMouseBtnCallback(
            [this](uint8_t btn, uint8_t action) { mouseButtonHook(btn, action); })
        .setMouseScrollCallback(
            [this](int8_t xOffset, int8_t yOffset) { mouseScrollHook(xOffset, yOffset); })
        .setWindowFileDropCallback(
            [this](int32_t count, const char** paths)
            {
                (void)count;
                (void)paths;
                for (int32_t i = 0; i < count; ++i) {}
            });

    // NativeWindow::enableScissorsTest(false);
    // NativeWindow::enableDepthTest(false);
}

UIWindow::~UIWindow()
{
    for (auto&[key, cursor] : cursors_)
    {
        glfwDestroyCursor(cursor);
    }
    cursors_.clear();
}

auto UIWindow::run() -> bool
{
    const glm::ivec2& size = window_.getSize();

    window_.makeContextCurrent();
    core::NativeWindow::updateViewport(size);
    core::NativeWindow::updateScissors({0, 0, size.x, size.y});
    core::NativeWindow::clearColor(utils::hexToVec4("#3d3d3dff"));
    core::NativeWindow::clearAllBuffers();

    processingQueue_.push(shared_from_this());
    while (!processingQueue_.empty())
    {
        UIBasePtr node = processingQueue_.front();
        processingQueue_.pop();

        if (areLayoutPreconditionsSatisfied(node))
        {
            preLayoutSetup(node);
            node->layout();
            postLayoutActions(node);
        }

        if (areRenderPreconditionsSatisfied(node))
        {
            preRenderSetup(node, projection_);
            node->render(projection_);
            postRenderActions(node);
        }

        for (const auto& childNode : node->getElements()) { processingQueue_.push(childNode); }
    }

    if (uiState_->wantedCursorType.has_value())
    {
        glfwSetCursor(window_.getGlfwHandle(), cursors_[uiState_->wantedCursorType.value()]);
        uiState_->currentCursorType = uiState_->wantedCursorType;
        uiState_->wantedCursorType.reset();
    }

    window_.swapBuffers();

    return window_.shouldWindowClose() || forcedQuit_;
}

auto UIWindow::quit() -> void { forcedQuit_ = true; }

auto UIWindow::render(const glm::mat4& projection) -> void {}

auto UIWindow::layout() -> void
{
    layoutBase_.setComputedScale(uiState_->windowSize);

    const auto& calculator = core::BasicCalculator::get();
    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForGenericElement(this);

    const auto overflow = calculator.calculateElementOverflow(this, {0, 0});
    calculator.calculateAlignmentForElements(this, overflow);
}

auto UIWindow::event(UIStatePtr& state) -> void
{
    using namespace core;
    const auto eId = state->currentEventId;
    if (eId == MouseEnterEvt::eventId)
    {
        MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        return eventsMgr_.emitEvent<MouseEnterEvt>(e);
    }
    else if (eId == MouseExitEvt::eventId)
    {
        MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        return eventsMgr_.emitEvent<MouseExitEvt>(e);
    }
    else if (eId == MouseLeftClickEvt::eventId)
    {
        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        MouseLeftClickEvt e{state->mousePos.x, state->mousePos.y};
        return eventsMgr_.emitEvent<MouseLeftClickEvt>(e);
    }
    else if (eId == MouseLeftReleaseEvt::eventId)
    {
        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        MouseLeftReleaseEvt e;
        return eventsMgr_.emitEvent<MouseLeftReleaseEvt>(e);
    }
}

auto UIWindow::windowResizeHook(const uint32_t x, const uint32_t y) -> void
{
    /* Note: use framebuffer size to set viewport in case DPI is not a default
       one aka we have some artificial scaling. */
    updateWindowSizeAndProjection(glm::ivec2{x, y});
    propagateEventTo(core::WindowResizeEvt{}, std::nullopt);
}

auto UIWindow::windowMouseEnterHook(const bool entered) -> void
{
    (void)entered;
    mouseMoveHook(uiState_->mousePos.x, uiState_->mousePos.y);
}

auto UIWindow::keyHook(const uint32_t key, const uint32_t scancode, const uint32_t action,
    const uint32_t mods) -> void
{
    using namespace core;
    if (action == Input::RELEASE || action == Input::REPEAT) { return; }
    if (key == Input::ESC)
    {
        window_.close();
    }
    else if (key == Input::C)
    {
        App::get().createWindow("new_frame" + std::to_string(id_), {200, 300});
    }
    else if (key == Input::P)
    {
        log_.debug("\n{}", shared_from_this());
    }
}

auto UIWindow::mouseMoveHook(const int32_t newX, const int32_t newY) -> void
{
    using namespace core;

    const glm::ivec2 newMouse = utils::clamp({newX, newY}, {0, 0}, window_.getSize());
    uint32_t prevHoveredId = uiState_->hoveredId;
    uiState_->hoveredId = node::NOTHING;
    uiState_->mouseDiff = newMouse - uiState_->mousePos;
    uiState_->mousePos = newMouse;

    propagateHoverScanEvent();
    uint32_t currHoveredId = uiState_->hoveredId;

    /* Entered the window for the first time */
    if (prevHoveredId == node::NOTHING)
    {
        propagateEventTo(MouseEnterEvt{}, currHoveredId);
    }
    /* Spawn exit for previous and enter for the current id */
    else if (prevHoveredId != currHoveredId)
    {
        uiState_->prevHoveredId = prevHoveredId;
        propagateEventTo(MouseEnterEvt{}, currHoveredId);
        propagateEventTo(MouseExitEvt{}, prevHoveredId);
    }

    /* Handle dragging on the clicked id */
    if (uiState_->clickedId != node::NOTHING
        && uiState_->mouseAction == Input::PRESS
        && uiState_->mouseButton == Input::LEFT)
    {
        uiState_->isDragging = true;
        propagateEventTo(MouseDragEvt{}, uiState_->clickedId);
    }

    /* Spawn event itself */
    propagateEventTo(MouseMoveEvt{}, std::nullopt);
}

auto UIWindow::mouseButtonHook(const uint32_t btn, const uint32_t action) -> void
{
    using namespace core;

    /* New rescan for the hovered id needs to be done as on button release/click the
    underlaying element could of got invalidated. */
    uiState_->hoveredZIndex = node::NOTHING;
    propagateHoverScanEvent();

    uiState_->mouseButton = static_cast<Input::Mouse>(btn);
    uiState_->mouseAction = static_cast<Input::Action>(action);
    propagateEventTo(MouseButtonEvt{}, std::nullopt);

    if (btn == Input::LEFT && action == Input::PRESS)
    {
        uiState_->clickedId = uiState_->hoveredId;
        uiState_->selectedId = uiState_->hoveredId;
        propagateEventTo(MouseLeftClickEvt{}, uiState_->clickedId);
    }
    else if (btn == Input::LEFT && action == Input::RELEASE)
    {
        uiState_->isDragging = false;
        uiState_->clickedId = node::NOTHING;
        propagateEventTo(MouseLeftReleaseEvt{}, uiState_->selectedId);
    }
}

auto UIWindow::mouseScrollHook(const uint32_t xOffset, const uint32_t yOffset) -> void
{
    uiState_->scrollOffset = {xOffset, yOffset};
    propagateEventTo(core::MouseScrollEvt{}, uiState_->closestScroll);
    uiState_->scrollOffset = {0, 0};
}

auto UIWindow::initializeDefaultCursors() -> void
{
    if (!isFirstWindow_) { return; }

    /* Create all available cursors at main window start. */
    using namespace core;
    cursors_[Input::Cursor::ARROW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursors_[Input::Cursor::IBEAM] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    cursors_[Input::Cursor::CROSSHAIR] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    cursors_[Input::Cursor::HAND] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    cursors_[Input::Cursor::HRESIZE] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    cursors_[Input::Cursor::VRESIZE] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    cursors_[Input::Cursor::ALLRESIZE] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    cursors_[Input::Cursor::NOT_ALLOWED] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);

    glfwSetCursor(window_.getGlfwHandle(), cursors_[Input::Cursor::ARROW]);
    isFirstWindow_ = false;
}

auto UIWindow::propagateEventTo(const core::IEvent& evt,
    const std::optional<uint32_t> nodeId) -> void
{
    uiState_->currentEventId = evt.getEventId();

    processingQueue_.push(shared_from_this());
    while (!processingQueue_.empty())
    {
        UIBasePtr node = processingQueue_.front();
        processingQueue_.pop();

        if (node->isIgnoringEvents()) { continue; }

        if (!nodeId || nodeId.value() == node->getId()) { node->event(uiState_); }

        for (const auto& childNode : node->getElements()) { processingQueue_.push(childNode); }
    }
}

auto UIWindow::updateWindowSizeAndProjection(const glm::ivec2 newSize) -> void
{
    uiState_->windowSizeDelta = newSize - uiState_->windowSize;
    uiState_->windowSize = newSize;

    /* Camera is looking into -Z by default. Here, higher Z means closer to the camera. */
    projection_ = glm::ortho(0.0f, (float)newSize.x, (float)newSize.y, 0.0f, -(float)MAX_LAYERS, 0.0f);
}

auto UIWindow::areRenderPreconditionsSatisfied(const UIBasePtr& node) -> bool
{
    //TODO: Do not render nodes that aint visible
    if (!node || !node->isParented()) { return false; }
    return true;
}

auto UIWindow::areLayoutPreconditionsSatisfied(const UIBasePtr& node) -> bool
{
    // Nothing big for now
    return true;
}

auto UIWindow::preRenderSetup(const UIBasePtr& node, const glm::mat4& projection) -> void
{
    const auto& nLayout = node->getBaseLayoutData();
    const auto& viewPos = nLayout.getViewPos();
    const auto& viewScale = nLayout.getViewScale();
    core::NativeWindow::updateScissors(
        {
            viewPos.x,
            std::round((-2.0f / projection[1][1])) - viewPos.y - viewScale.y,
            viewScale.x,
            viewScale.y
        });
}

auto UIWindow::preLayoutSetup(const UIBasePtr& node) -> void
{
    /* If is the root window element or dropdown, scissor area is the whole node area. */
    // if (node->getTypeId() == UIWindow::typeId || node->getTypeId() == UIDropdown::typeId)
    if (node->getTypeId() == UIWindow::typeId)
    {
        auto& nLayout = node->getBaseLayoutData();
        nLayout.setViewPos(nLayout.getComputedPos());
        nLayout.setViewScale(nLayout.getComputedScale());
    }
}

auto UIWindow::propagateHoverScanEvent() -> void
{
    uint32_t maxZIndex{0};

    processingQueue_.push(shared_from_this());
    while (!processingQueue_.empty())
    {
        UIBasePtr node = processingQueue_.front();
        processingQueue_.pop();

        if (node->isIgnoringEvents()) { continue; }

        /* Determine in the scan pass who's the hovered element. We need to ensure that the user's input will
            go to the highest index element. */
        if (node->layoutBase_.getZIndex() > maxZIndex && node->layoutBase_.isPointInsideView(uiState_->mousePos))
        {
            uiState_->hoveredId = node->getId();
            maxZIndex = node->layoutBase_.getZIndex();
        }

        for (const auto& childNode : node->getElements()) { processingQueue_.push(childNode); }
    }
}

auto UIWindow::postRenderActions(const UIBasePtr& node) -> void
{
    // Nothing big for now
}

auto UIWindow::postLayoutActions(const UIBasePtr& node) -> void
{
    /* After calculating myself , compute how much of them is still visible inside of the parent.
        The elements of a dropdown will always be fully visible aka their view scale and pos is their
        actual computed scale and pos. */
    std::ranges::for_each(node->getElements(),
        [&node](const auto& it)
        {
            auto& itLayout = it->getBaseLayoutData();
            const auto& nodeLayout = node->getBaseLayoutData();
            itLayout.computeViewBox(node->getBaseLayoutData());

            /* Index is used for layer rendering order. Can be custom. Otherwise it is just 1 + parentIndex. */
            if (!itLayout.isCustomIndex())
            {
                itLayout.setZIndex(nodeLayout.getZIndex() + 1);
            }

            // /* It's a pane scrollbar and it will have a higher custom zIndex */
            // if (e->getTypeId() == UISlider::typeId && e->getCustomTagId() == UISlider::scrollTagId)
            // {
            //     e->setIndex(UISlider::scrollIndexOffset - getIndex());
            // }

            /* Depth is used mostly for printing. */
            it->depth_ = node->depth_ + 1;
        });
}

auto UIWindow::setTitle(std::string title, const bool onlyForShow) -> void
{
    window_.setTitle(std::move(title), onlyForShow);
}

auto UIWindow::getDeltaTime() -> double { return window_.getDeltaTime(); }

auto UIWindow::getTitle() -> std::string { return window_.getTitle(); }

auto UIWindow::getWindow() -> core::NativeWindow& { return window_; }

auto UIWindow::isMainWindow() -> bool { return isMainWindow_; }

/* Static declarations */
int32_t UIWindow::MAX_LAYERS = 1000;
bool UIWindow::isFirstWindow_ = true;
std::unordered_map<core::Input::Cursor, GLFWcursor*> UIWindow::cursors_ = {};
} // namespace lav::node