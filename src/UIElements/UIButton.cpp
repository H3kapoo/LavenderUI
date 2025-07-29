#include "UIButton.hpp"

#include "src/ElementComposable/IEvent.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/LayoutCalculator/BasicCalculator.hpp"

namespace src::uielements
{
UIButton::UIButton() : UIBase(getTypeInfo())
{
    setScale({100_px, 36_px});
}

auto UIButton::render(const glm::mat4& projection) -> void
{
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", getTransform());
    shader_.uploadVec4f("uColor", getColor());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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

    renderNext(projection);
}

auto UIButton::layout() -> void
{
    using namespace layoutcalculator;
    // BasicCalculator::get().calcElementsPos(shared_from_this());

    // log_.debug("Doing layout for {} {}", id_, UIButton::typeId);

    /* Position the text */
    const glm::vec2 p = getComputedPos() + getComputedScale() / 2.0f
        - textAttribs_.computeMaxSize() / 2.0f;
    textAttribs_.setPosition(p);
}

auto UIButton::event(state::UIWindowStatePtr& state) -> void
{
    using namespace elementcomposable;
    /* Let the base do the generic stuff */
    UIBase::event(state);

    const auto eId = state->currentEventId;
    if (eId == MouseButtonEvt::eventId && state->hoveredId == id_)
    {
        MouseButtonEvt e{state->mouseButton, state->mouseAction};
        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        return emitEvent<MouseButtonEvt>(e);
    }
    else if (eId == MouseEnterEvt::eventId && state->hoveredId == id_)
    {
        MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        return emitEvent<MouseEnterEvt>(e);
    }
    else if (eId == MouseExitEvt::eventId && state->prevHoveredId == id_)
    {
        MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        return emitEvent<MouseExitEvt>(e);
    }

    eventNext(state);
}

auto UIButton::setFont(const std::filesystem::path& fontPath) -> void
{

}

auto UIButton::setText(const std::string& text) -> void { textAttribs_.setText(text); }
} // namespace src::uielements
