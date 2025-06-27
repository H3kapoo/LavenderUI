#include "UIFrame.hpp"

#include "src/App.hpp"
#include "src/FrameState/FrameState.hpp"
#include "src/ElementEvents/IEvent.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/Utils/Misc.hpp"
#include "vendor/glm/ext/matrix_clip_space.hpp"

namespace src::uielements
{
using namespace windowmanagement;

bool UIFrame::isFirstFrame_ = true;

UIFrame::UIFrame(const std::string& title, const glm::ivec2& size)
    : window_(title, size)
    , frameState_(utils::make<framestate::FrameState>())
    , forcedQuit_(false)
    , isMainFrame_(isFirstFrame_)
{
    isFirstFrame_ = false;

    updateProjection();

    /* Note: use framebuffer size to set viewport in case DPI is not a default
       one aka we have some artificial scaling. */
    window_.getInput().setWindowSizeCallback([this](uint32_t x, uint32_t y)
    {
        updateProjection();
    });

    window_.getInput().setKeyCallback([this](auto key, auto sc, auto action, auto mods)
    {
        if (action == GLFW_RELEASE || action == GLFW_REPEAT) { return; }
        log_.debug("window: {} | {} {} {} {}", window_.getId(), key, sc, action, mods);
        if (key == GLFW_KEY_ESCAPE)
        {
            window_.doCloseWindow();
        }
        else if (key == GLFW_KEY_C)
        {
            App::get().createFrame("new_frame" + std::to_string(id_), {200, 300});
        }
        else if (key == GLFW_KEY_P)
        {
            std::println("{}", shared_from_this());
        }
    });

    window_.getInput().setMouseMoveCallback([this](int32_t x, int32_t y)
    {
        using namespace elementevents;
        frameState_->mousePos = {x, y};

        uint32_t prevHoveredId = frameState_->hoveredId;
        event(frameState_, MouseMoveScanEvt{});
        uint32_t currentHoveredId = frameState_->hoveredId;

        if (prevHoveredId == framestate::NOTHING)
        {
            event(frameState_, MouseEnterEvt{});
        }
        else if (prevHoveredId != 0 && currentHoveredId != 0 && prevHoveredId != currentHoveredId)
        {
            frameState_->prevHoveredId = prevHoveredId;
            event(frameState_, MouseEnterEvt{});
            event(frameState_, MouseExitEvt{});
        }

        event(frameState_, MouseMoveEvt{});
    });

    window_.getInput().setMouseBtnCallback([this](uint8_t btn, uint8_t action)
    {
        using namespace elementevents;
        frameState_->mouseButton = btn;
        frameState_->mouseAction = action;
        event(frameState_, MouseButtonEvt{});
    });
}

auto UIFrame::run() -> bool
{
    const glm::ivec2& size = window_.getSize();

    window_.makeContextCurrent();

    NativeWindow::updateViewport(size);
    NativeWindow::updateScissors({0, 0, size.x, size.y});
    NativeWindow::clearColor(utils::hexToVec4("#3d3d3dff"));
    NativeWindow::clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    layoutAttr_.scale = size;

    layout();
    render(projection_);

    window_.swapBuffers();

    return getWindow().shouldWindowClose() || forcedQuit_;
}

auto UIFrame::render(const glm::mat4& projection) -> void
{
    /* UIFrame has a mesh available if need be. For now no need to render anything, skip to rendering
        child elements. */
    renderNext(projection);
}

auto UIFrame::layout() -> void
{
    layoutNext();
}

auto UIFrame::event(framestate::FrameStatePtr& state, const elementevents::IEvent& e) -> void
{
    /* Do default handling of events. */
    UIBase::event(state, e);

    /* Bubble event down the tree to the leafs. */
    eventNext(state, e);
}

auto UIFrame::updateProjection() -> void
{
    /* Camera is looking into -Z by default. Here, higher Z means closer to the camera. */
    const glm::ivec2 size = window_.getSize();
    projection_ = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, -1000.0f, 0.0f);
}

auto UIFrame::getWindow() -> NativeWindow& { return window_; }

auto UIFrame::isMainFrame() -> bool { return isMainFrame_; }
} // namespace src::uielements