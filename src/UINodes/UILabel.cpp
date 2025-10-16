#include "UILabel.hpp"

#include <optional>

#include "src/ElementComposable/IEvent.hpp"
#include "src/Utils/Misc.hpp"
#include "src/WindowManagement/Input.hpp"

namespace src::uinodes
{
using namespace elementcomposable;
using namespace windowmanagement;

UILabel::UILabel() : UIBase({"UILabel", "elemVert.glsl", "elemFrag.glsl"})
{
    layoutBase_.setScale({100_px, 36_px});
    setIgnoreEvents();
}

UILabel::UILabel(UIBaseInitData&& data) : UIBase(std::move(data))
{}

auto UILabel::render(const glm::mat4& projection) -> void
{
    // mesh_.bind();
    // shader_.bind();
    // shader_.uploadMat4("uMatrixProjection", projection);
    // shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    // shader_.uploadVec4f("uColor", overrideColor_ ? *overrideColor_ : getColor());
    // shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    // shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    // shader_.uploadVec4f("uBorderRadii", layoutBase_.getBorderRadius());
    // shader_.uploadVec4f("uBorderColor", getBorderColor());
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    /* Draw the text */
    const auto& textShader_ = textAttribs_.getShader();
    const auto& textBuffer = textAttribs_.getBuffer();
    textShader_.bind();
    textShader_.uploadVec4f("uColor", utils::hexToVec4("#141414ff"));
    textShader_.uploadTexture2DArray("uTextureArray", GL_TEXTURE0, textAttribs_.getFont()->textureId);
    textShader_.uploadMat4("uMatrixProjection", projection);
    textShader_.uploadMat4v("uModelMatrices", textBuffer.model);
    textShader_.uploadIntv("uCharIndices", textBuffer.glyphCode);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, textAttribs_.getText().size());
}

auto UILabel::layout() -> void
{
    const glm::vec2 p = layoutBase_.getComputedPos() + layoutBase_.getComputedScale() / 2.0f
        - textAttribs_.computeMaxSize() / 2.0f;
    textAttribs_.setPosition(p);
}

auto UILabel::event(state::UIStatePtr& state) -> void
{
    const auto eId = state->currentEventId;
    if (eId == MouseLeftClickEvt::eventId)
    {
        MouseLeftClickEvt e{state->mousePos.x, state->mousePos.y};
        return eventsMgr_.emitEvent<MouseLeftClickEvt>(e);
    }
    else if (eId == MouseLeftReleaseEvt::eventId)
    {
        MouseLeftReleaseEvt e;
        return eventsMgr_.emitEvent<MouseLeftReleaseEvt>(e);
    }
    else if (eId == MouseDragEvt::eventId)
    {
        MouseDragEvt e{state->mousePos.x, state->mousePos.y};
        return eventsMgr_.emitEvent<MouseDragEvt>(e);
    }
    else if (eId == MouseEnterEvt::eventId)
    {
        MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        return eventsMgr_.emitEvent<MouseEnterEvt>(e);
    }
    else if (eId == MouseExitEvt::eventId)
    {
        MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        return eventsMgr_.emitEvent<MouseExitEvt>(e);
    }
}

auto UILabel::setColor(const glm::vec4& value) -> UILabel& { baseColor_ = value; return *this; }
auto UILabel::setText(const std::string& text) -> UILabel& { textAttribs_.setText(text); return *this; }
auto UILabel::setFont(const std::filesystem::path& fontPath) -> void {}

auto UILabel::getColor() const -> const glm::vec4& { return baseColor_; }
auto UILabel::getBorderColor() const -> const glm::vec4& { return borderColor_; }
} // namespace src::uinodes
