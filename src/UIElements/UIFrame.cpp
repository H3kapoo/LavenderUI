#include "src/UIElements/UIFrame.hpp"
#include "src/App.hpp"
#include "src/UIElements/UIBase.hpp"
#include "vendor/glm/ext/matrix_clip_space.hpp"

namespace src::uielements
{
using namespace windowmanagement;

bool UIFrame::isFirstFrame_ = true;

UIFrame::UIFrame(const std::string& title, const glm::ivec2& size)
    : UIBase("UIFrame", typeid(UIFrame))
    , window_(title, size)
    , forcedQuit_(false)
    , isMainFrame_(isFirstFrame_)
{
    isFirstFrame_ = false;

    updateProjection();

    // layout_.pos.z = 1;
    // layout_.pos.x = 100;
    // layout_.pos.y = 100;

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
    });
}

auto UIFrame::run() -> bool
{
    const glm::ivec2& size = window_.getSize();

    window_.makeContextCurrent();

    NativeWindow::updateViewport(size);
    NativeWindow::updateScissors({0, 0, size.x, size.y});
    NativeWindow::clearColor({1.0f, 0.4f, 1.0f, 1.0f});
    NativeWindow::clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    layout_.pos.z = 1;
    // layout_.pos.z = 2;
    layout_.pos.x = 100;
    layout_.pos.y = 100;

    render(projection_);
    window_.swapBuffers();

    return getWindow().shouldWindowClose() || forcedQuit_;
}

auto UIFrame::render(const glm::mat4& projection) -> void
{
    NativeWindow::updateScissors(
    {
        layout_.pos.x,
        std::floor((-2.0f / projection[1][1])) - layout_.pos.y - layout_.scale.y,
        layout_.scale.x,
        layout_.scale.y
    });

    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layout_.getTransform());
    // shader_.uploadVec4f("uColor", vAttribs_.color);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

auto UIFrame::layout() -> void
{
}

auto UIFrame::updateProjection() -> void
{
    /* Camera is looking into -Z by default */
    const glm::ivec2 size = window_.getSize();
    projection_ = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, -1000.0f, 0.0f);
}

auto UIFrame::getWindow() -> NativeWindow& { return window_; }

auto UIFrame::isMainFrame() -> bool { return isMainFrame_; }
} // namespace src::uielements