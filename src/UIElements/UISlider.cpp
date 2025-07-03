#include "UISlider.hpp"

#include "src/ElementComposable/LayoutAttribs.hpp"
#include "src/ElementEvents/IEvent.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/Utils/Misc.hpp"
#include "vendor/glfw/include/GLFW/glfw3.h"

namespace src::uielements
{
UISlider::UISlider()
{
    knobVisualAttr_.color = utils::hexToVec4("#ca5555ff");
}

auto UISlider::render(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutAttr_.getTransform());
    shader_.uploadVec4f("uColor", visualAttr_.color);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    /* Draw knob */
    // mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", knobLayoutAttr_.getTransform());
    shader_.uploadVec4f("uColor", knobVisualAttr_.color);
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
    knobLayoutAttr_.index = layoutAttr_.index + 1;

    if (layoutAttr_.type == LayoutAttribs::Type::HORIZONTAL)
    {
        knobLayoutAttr_.cScale = {layoutAttr_.cScale.y, layoutAttr_.cScale.y};
    }
    else if (layoutAttr_.type == LayoutAttribs::Type::VERTICAL)
    {
        knobLayoutAttr_.cScale = {layoutAttr_.cScale.x, layoutAttr_.cScale.x};
    }

    calculateKnobPosition();

    /* Position the text */
    textAttribs_.setPosition(knobLayoutAttr_.cPos + 10.0f);
}

auto UISlider::event(framestate::FrameStatePtr& state) -> void
{
    using namespace elementevents;

    /* Let the base do the generic stuff like mouse move pre-pass. */
    UIBase::event(state);

    const auto eId = state->currentEventId;
    if (eId == MouseScrollEvt::eventId && state->hoveredId == id_)
    {
        percentage_ += state->scrollOffset.y * 0.1f;
        percentage_ = std::clamp(percentage_, 0.0f, 1.0f);

        SliderEvt sliderEvt{getScrollValue()};
        eventManager_.emit<SliderEvt>(sliderEvt);
    }
    else if (eId == MouseDragEvt::eventId && state->clickedId == id_)
    {
        percentage_ = calculatePercentage(state->mousePos);

        SliderEvt sliderEvt{getScrollValue()};
        eventManager_.emit<SliderEvt>(sliderEvt);

        /* We can safely ignore bubbling down the tree as we found the dragged element. */
        MouseDragEvt e;
        return eventManager_.emit<MouseDragEvt>(e);
    }
    else if (eId == MouseButtonEvt::eventId && state->hoveredId == id_)
    {
        if (state->mouseButton == GLFW_MOUSE_BUTTON_LEFT && state->mouseAction == GLFW_PRESS)
        {
            percentage_ = calculatePercentage(state->mousePos);

            SliderEvt sliderEvt{getScrollValue()};
            eventManager_.emit<SliderEvt>(sliderEvt);
        }

        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        MouseButtonEvt e{state->mouseButton, state->mouseAction};
        return eventManager_.emit<MouseButtonEvt>(e);
    }
    else if (eId == MouseEnterEvt::eventId && state->hoveredId == id_)
    {
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        return eventManager_.emit<MouseEnterEvt>(e);
    }
    else if (eId == MouseExitEvt::eventId && state->prevHoveredId == id_)
    {
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        return eventManager_.emit<MouseExitEvt>(e);
    }
}

auto UISlider::calculatePercentage(const glm::ivec2& mPos) -> float
{
    if (layoutAttr_.type == LayoutAttribs::Type::HORIZONTAL)
    {
        const float halfKnobScale = knobLayoutAttr_.cScale.x / 2.0f;
        return utils::remap(mPos.x,
            layoutAttr_.cPos.x + halfKnobScale, layoutAttr_.cPos.x + layoutAttr_.cScale.x - halfKnobScale,
            0.0f, 1.0f);
    }
    else if (layoutAttr_.type == LayoutAttribs::Type::VERTICAL)
    {
        const float halfKnobScale = knobLayoutAttr_.cScale.y / 2.0f;
        return 1.0f - utils::remap(mPos.y,
            layoutAttr_.cPos.y + halfKnobScale, layoutAttr_.cPos.y + layoutAttr_.cScale.y - halfKnobScale,
            0.0f, 1.0f);
    }

    return 0.0f;
}

auto UISlider::calculateKnobPosition() -> void
{
    if (layoutAttr_.type == LayoutAttribs::Type::HORIZONTAL)
    {
        knobLayoutAttr_.cPos.y = layoutAttr_.cPos.y;
        knobLayoutAttr_.cPos.x = utils::remap(percentage_,
            0.0f, 1.0f, layoutAttr_.cPos.x,
            layoutAttr_.cPos.x + layoutAttr_.cScale.x - knobLayoutAttr_.cScale.x);
    }
    else if (layoutAttr_.type == LayoutAttribs::Type::VERTICAL)
    {
        knobLayoutAttr_.cPos.x = layoutAttr_.cPos.x;
        knobLayoutAttr_.cPos.y = utils::remap(1.0f - percentage_,
            0.0f, 1.0f, layoutAttr_.cPos.y,
            layoutAttr_.cPos.y + layoutAttr_.cScale.y - knobLayoutAttr_.cScale.y);
    }
}

auto UISlider::getKnobLayout() -> LayoutAttribs&
{
    return knobLayoutAttr_;
}

auto UISlider::getKnobVisual() -> VisualAttribs&
{
    return knobVisualAttr_;
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
} // namespace src::uielements
