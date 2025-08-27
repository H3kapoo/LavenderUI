#include "UIWindow.hpp"

#include "src/App.hpp"
#include "src/ElementComposable/IEvent.hpp"
#include "src/State/UIWindowState.hpp"
#include "src/LayoutCalculator/BasicCalculator.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/Utils/Misc.hpp"
#include "src/WindowManagement/Input.hpp"
#include "src/WindowManagement/NativeWindow.hpp"
#include "vendor/glm/ext/matrix_clip_space.hpp"

namespace src::uielements
{
using namespace windowmanagement;
using namespace elementcomposable;

bool UIWindow::isFirstWindow_ = true;
int32_t UIWindow::MAX_LAYERS = 1000;

UIWindow::UIWindow(const std::string& title, const glm::ivec2& size)
    : UIBase(getTypeInfo())
    , window_(title, size)
    , isMainWindow_(isFirstWindow_)
{
    isFirstWindow_ = false;

    windowState_->windowSize = window_.getSize();

    updateProjection();

    /* Setup hooks into events */
    window_.getInput().setWindowSizeCallback(
        [this](uint32_t x, uint32_t y) { windowResizeHook(x, y); });

    window_.getInput().setWindowMouseEnterCallback(
        [this](bool entered) { windowMouseEnterHook(entered); });

    window_.getInput().setWindowFileDropCallback(
        [this](int32_t count, const char** paths)
        {
            (void)count;
            (void)paths;
            for (int32_t i = 0; i < count; ++i) {}
        });

    window_.getInput().setCharacterCallback(
        [this](uint32_t cp){ (void)cp; });

    window_.getInput().setKeyCallback(
        [this](uint32_t key, uint32_t sc, uint32_t action, uint32_t mods) { keyHook(key, sc, action, mods); });

    window_.getInput().setMouseMoveCallback(
        [this](int32_t x, int32_t y) { mouseMoveHook(x, y); });

    window_.getInput().setMouseBtnCallback(
        [this](uint8_t btn, uint8_t action) { mouseButtonHook(btn, action); });

    window_.getInput().setMouseScrollCallback(
        [this](int8_t xOffset, int8_t yOffset) { mouseScrollHook(xOffset, yOffset); });

    // NativeWindow::enableScissorsTest(false);
    // NativeWindow::enableDepthTest(false);
}

auto UIWindow::run() -> bool
{
    const glm::ivec2& size = window_.getSize();

    window_.makeContextCurrent();
    NativeWindow::updateViewport(size);
    NativeWindow::updateScissors({0, 0, size.x, size.y});
    NativeWindow::clearColor(utils::hexToVec4("#3d3d3dff"));
    NativeWindow::clearAllBuffers();

    setComputedScale(size);

    layout();
    render(projection_);

    window_.swapBuffers();

    return window_.shouldWindowClose() || forcedQuit_;
}

auto UIWindow::render(const glm::mat4& projection) -> void
{
    /* UIWindow has a mesh available if need be. For now no need to render anything, skip to rendering
        child elements. */
    renderNext(projection);
}

auto UIWindow::layout() -> void
{
    using namespace layoutcalculator;
    const auto& calculator = BasicCalculator::get();
    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForGenericElement(this);

    layoutNext();
}

auto UIWindow::event(state::UIWindowStatePtr& state) -> void
{
    /* Do mandatory handling of base events. */
    UIBase::event(state);

    /* Bubble event down the tree. */
    eventNext(state);
}

auto UIWindow::windowResizeHook(const uint32_t x, const uint32_t y) -> void
{
    /* Note: use framebuffer size to set viewport in case DPI is not a default
       one aka we have some artificial scaling. */
    updateProjection();
    windowState_->windowSizeDelta = glm::ivec2{x, y} - windowState_->windowSize;
    windowState_->windowSize = {x, y};
    spawnEvent(WindowResizeEvt{});
}

auto UIWindow::windowMouseEnterHook(const bool entered) -> void
{
    if (entered)
    {
        mouseMoveHook(windowState_->mousePos.x, windowState_->mousePos.y);
    }
    else
    {
        mouseMoveHook(windowState_->mousePos.x, windowState_->mousePos.y);

        // mouseMoveHook(-1, -1);
    }
}

auto UIWindow::keyHook(const uint32_t key, const uint32_t scancode, const uint32_t action,
    const uint32_t mods) -> void
{
    if (action == Input::RELEASE || action == Input::REPEAT) { return; }
    log_.debug("window: {} | {} {} {} {}", window_.getId(), key, scancode, action, mods);
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
        std::println("{}", shared_from_this());
    }
}

auto UIWindow::mouseMoveHook(const int32_t newX, const int32_t newY) -> void
{
    const glm::ivec2 newMouse = utils::clamp({newX, newY}, {0, 0}, window_.getSize());
    // const glm::ivec2 newMouse{newX, newY};
    uint32_t prevHoveredId = windowState_->hoveredId;
    windowState_->hoveredId = state::NOTHING;
    windowState_->hoveredZIndex = state::NOTHING;
    windowState_->mouseDiff = newMouse - windowState_->mousePos;
    windowState_->mousePos = newMouse;

    spawnEvent(MouseMoveScanEvt{});
    uint32_t currentHoveredId = windowState_->hoveredId;

    /* Entered the window for the first time */
    if (prevHoveredId == state::NOTHING)
    {
        spawnEvent(MouseEnterEvt{});
    }
    /* Spawn exit for previous and enter for the current id */
    else if (prevHoveredId != currentHoveredId)
    {
        windowState_->prevHoveredId = prevHoveredId;
        spawnEvent(MouseEnterEvt{});
        spawnEvent(MouseExitEvt{});
    }

    /* Handle dragging on the clicked id */
    if (windowState_->clickedId != state::NOTHING
        && windowState_->mouseAction == Input::PRESS
        && windowState_->mouseButton == Input::LEFT)
    {
        windowState_->isDragging = true;
        spawnEvent(MouseDragEvt{});
    }

    /* Spawn event itself */
    spawnEvent(MouseMoveEvt{});
}

auto UIWindow::mouseButtonHook(const uint32_t btn, const uint32_t action) -> void
{
    /* New rescan for the hovered id needs to be done as on button release/click the
    underlaying element could of got invalidated. */
    windowState_->hoveredZIndex = state::NOTHING;
    spawnEvent(MouseMoveScanEvt{});

    windowState_->mouseButton = btn;
    windowState_->mouseAction = action;
    spawnEvent(MouseButtonEvt{});

    if (btn == Input::LEFT && action == Input::PRESS)
    {
        windowState_->clickedId = windowState_->hoveredId;
        windowState_->selectedId = windowState_->hoveredId;
        spawnEvent(MouseLeftClickEvt{});
    }
    else if (btn == Input::LEFT && action == Input::RELEASE)
    {
        windowState_->isDragging = false;
        windowState_->clickedId = state::NOTHING;
        spawnEvent(MouseLeftReleaseEvt{});
    }
}

auto UIWindow::mouseScrollHook(const uint32_t xOffset, const uint32_t yOffset) -> void
{
    windowState_->scrollOffset = {xOffset, yOffset};
    spawnEvent(MouseScrollEvt{});
    windowState_->scrollOffset = {0, 0};
}

auto UIWindow::spawnEvent(const elementcomposable::IEvent& e) -> void
{
    windowState_->currentEventId = e.getEventId();
    event(windowState_);
}

auto UIWindow::updateProjection() -> void
{
    /* Camera is looking into -Z by default. Here, higher Z means closer to the camera. */
    const glm::ivec2 size = window_.getSize();
    projection_ = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, -(float)MAX_LAYERS, 0.0f);
}

auto UIWindow::setTitle(std::string title, const bool onlyForShow) -> void
{
    window_.setTitle(std::move(title), onlyForShow);
}

auto UIWindow::getDeltaTime() -> double { return window_.getDeltaTime(); }

auto UIWindow::getTitle() -> std::string { return window_.getTitle(); }

auto UIWindow::getWindow() -> NativeWindow& { return window_; }

auto UIWindow::isMainWindow() -> bool { return isMainWindow_; }
} // namespace src::uielements