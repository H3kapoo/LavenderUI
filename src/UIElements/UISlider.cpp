#include "UISlider.hpp"

#include "src/ElementComposable/LayoutBase.hpp"
#include "src/ElementComposable/IEvent.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/Utils/Misc.hpp"
#include "vendor/glfw/include/GLFW/glfw3.h"

namespace src::uielements
{
UISlider::UISlider() : UIBase(getTypeInfo()) 
{
    knobProps_.setColor(utils::hexToVec4("#ca5555ff"));
}

auto UISlider::render(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", propsBase_.getColor());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    /* Draw knob */
    // mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", knobLayout_.getTransform());
    shader_.uploadVec4f("uColor", knobProps_.getColor());
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

auto UISlider::layout() -> void
{
    /* Technically this node cannot have child elems. Do your layout logic here.*/
    knobLayout_.setIndex(layoutBase_.getIndex() + 1);

    const auto& computedScale = layoutBase_.getComputedScale();
    if (layoutBase_.getType() == LayoutBase::Type::HORIZONTAL)
    {
        knobLayout_.setComputedScale({
            std::max(computedScale.y, computedScale.x - scrollTo_), computedScale.y});
    }
    else if (layoutBase_.getType() == LayoutBase::Type::VERTICAL)
    {
        knobLayout_.setComputedScale({computedScale.x,
            std::max(computedScale.x, computedScale.y - scrollTo_)});
    }

    calculateKnobPosition();

    /* Position the text */
    const glm::vec2 p = knobLayout_.getComputedPos() + knobLayout_.getComputedScale() / 2.0f
        - textAttribs_.computeMaxSize() / 2.0f;
    textAttribs_.setPosition(p);
}

auto UISlider::event(framestate::FrameStatePtr& state) -> void
{
    /* Let the base do the generic stuff like mouse move pre-pass. */
    UIBase::event(state);

    const auto eId = state->currentEventId;
    if (eId == MouseScrollEvt::eventId && (state->hoveredId == id_ || state->closestScroll == id_))
    {
        // NOTE: inverting affects horizontal sliders. No side effects really.
        percentage_ += state->scrollOffset.y * sensitivity_ * (invertVertical_ ? -1 : 1);
        percentage_ = std::clamp(percentage_, 0.0f, 1.0f);

        /* We can safely ignore bubbling down the tree as we found the slided element. */
        SliderEvt sliderEvt{getScrollValue()}; 
        return events_.emitEvent<SliderEvt>(sliderEvt);
    }
    else if (eId == MouseDragEvt::eventId && state->clickedId == id_)
    {
        percentage_ = calculatePercentage(state->mousePos - offsetToKnobCenter_);

        SliderEvt sliderEvt{getScrollValue()};
        events_.emitEvent<SliderEvt>(sliderEvt);

        /* We can safely ignore bubbling down the tree as we found the dragged element. */
        MouseDragEvt e;
        return events_.emitEvent<MouseDragEvt>(e);
    }
    else if (eId == MouseButtonEvt::eventId && state->hoveredId == id_)
    {
        if (state->mouseButton == GLFW_MOUSE_BUTTON_LEFT && state->mouseAction == GLFW_PRESS)
        {
            const glm::vec2 knobHalf = knobLayout_.getComputedScale() / 2.0f;
            const glm::ivec2 middle = knobLayout_.getComputedPos() + knobHalf;
            offsetToKnobCenter_ = state->mousePos - middle;
            offsetToKnobCenter_.x = std::abs(offsetToKnobCenter_.x) > knobHalf.x
                ? 0.0f : offsetToKnobCenter_.x;
            offsetToKnobCenter_.y = std::abs(offsetToKnobCenter_.y) > knobHalf.y
                ? 0.0f : offsetToKnobCenter_.y;
            percentage_ = calculatePercentage(state->mousePos - offsetToKnobCenter_);

            SliderEvt sliderEvt{getScrollValue()};
            events_.emitEvent<SliderEvt>(sliderEvt);
        }

        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        MouseButtonEvt e{state->mouseButton, state->mouseAction};
        return events_.emitEvent<MouseButtonEvt>(e);
    }
    else if (eId == MouseEnterEvt::eventId && state->hoveredId == id_)
    {
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        return events_.emitEvent<MouseEnterEvt>(e);
    }
    else if (eId == MouseExitEvt::eventId && state->prevHoveredId == id_)
    {
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        return events_.emitEvent<MouseExitEvt>(e);
    }
}

auto UISlider::calculatePercentage(const glm::ivec2& mPos) -> float
{
    const auto& computedPos = layoutBase_.getComputedPos();
    const auto& computedScale = layoutBase_.getComputedScale();
    if (layoutBase_.getType() == LayoutBase::Type::HORIZONTAL)
    {
        const float halfKnobScale = knobLayout_.getComputedScale().x / 2.0f;
        return utils::remap(mPos.x,
            computedPos.x + halfKnobScale, computedPos.x + computedScale.x - halfKnobScale,
            0.0f, 1.0f);
    }
    else if (layoutBase_.getType() == LayoutBase::Type::VERTICAL)
    {
        const float halfKnobScale = knobLayout_.getComputedScale().y / 2.0f;
        const float remapped = utils::remap(mPos.y,
            computedPos.y + halfKnobScale, computedPos.y + computedScale.y - halfKnobScale,
            0.0f, 1.0f);
        return invertVertical_ ? remapped : 1.0f - remapped;
    }

    return 0.0f;
}

auto UISlider::calculateKnobPosition() -> void
{
    const auto& computedPos = layoutBase_.getComputedPos();
    const auto& computedScale = layoutBase_.getComputedScale();
    if (layoutBase_.getType() == LayoutBase::Type::HORIZONTAL)
    {
        knobLayout_.setComputedPos({
            utils::remap(percentage_, 0.0f, 1.0f,
                computedPos.x, computedPos.x + computedScale.x - knobLayout_.getComputedScale().x),
                computedPos.y});
    }
    else if (layoutBase_.getType() == LayoutBase::Type::VERTICAL)
    {
        knobLayout_.setComputedPos({computedPos.x,
            utils::remap(invertVertical_ ? percentage_ : 1.0f - percentage_, 0.0f, 1.0f,
                computedPos.y, computedPos.y + computedScale.y - knobLayout_.getComputedScale().y)});
    }
}

auto UISlider::getKnobLayout() -> LayoutBase&
{
    return knobLayout_;
}

auto UISlider::getKnobProps() -> PropsBase&
{
    return knobProps_;
}

auto UISlider::getTextAttribs() -> TextAttribs&
{
    return textAttribs_;
}

auto UISlider::getScrollPercentage() -> float
{
    return percentage_;
}

auto UISlider::getScrollValue() -> float
{
    scrollValue_ = utils::remap(percentage_, 0.0f, 1.0f, scrollFrom_, scrollTo_); 
    return scrollValue_;
}

auto UISlider::setScrollValue(const float value) -> void
{
    percentage_ = utils::remap(value, scrollFrom_, scrollTo_, 0.0f, 1.0f);
}

auto UISlider::setScrollFrom(const float value) -> void
{
    scrollFrom_ = value;
}

auto UISlider::setScrollTo(const float value) -> void
{
    scrollTo_ = value;
}

auto UISlider::setScrollSensitivity(const float value) -> void
{
    sensitivity_ = value;
}

auto UISlider::setText(const std::string& text) -> void
{
    // sensitivity_ = value;
    textAttribs_.setText(text);
}

auto UISlider::enableVerticalInversion(const bool value) -> void
{
    invertVertical_ = value;
}
} // namespace src::uielements
