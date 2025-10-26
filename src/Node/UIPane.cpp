#include "UIPane.hpp"

#include "src/ElementComposable/IEvent.hpp"
#include "src/LayoutCalculator/BasicCalculator.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "src/UIElements/UISlider.hpp"
#include "src/Utils/Misc.hpp"

namespace src::uielements
{
using namespace layoutcalculator;

UIPane::UIPane() : UIPane(getTypeInfo())
{}

UIPane::UIPane(const std::type_index& type)
    : UIBase(type)
{}

auto UIPane::render(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", getTransform());
    shader_.uploadVec4f("uColor", getColor());
    shader_.uploadVec2f("uResolution", getComputedScale());
    shader_.uploadVec4f("uBorderSize", {getBorder().top, getBorder().bot, getBorder().left, getBorder().right});
    shader_.uploadVec4f("uBorderColor", getBorderColor());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    static auto l = [](const UIBasePtr& e) { return e->getCustomTagId () == UISlider::scrollTagId; };
    renderNextExcept(projection, l);
    renderNextSingle(projection, vSlider_);
    renderNextSingle(projection, hSlider_);
}

auto UIPane::layout() -> void
{
    const auto& calculator = BasicCalculator::get();

    updateTriesCount_ = 0;
    glm::ivec2 overflow{0, 0};
    do
    {
        const auto sliderImpact = calculator.calculateSlidersScaleAndPos(this);
        calculator.calculateScaleForGenericElement(this, sliderImpact);
        calculator.calculatePositionForGenericElement(this, sliderImpact);

        overflow = calculator.calculateElementOverflow(this, sliderImpact);
        calculator.calculateAlignmentForElements(this, overflow);

        ++updateTriesCount_;
        /* Adding new elements (slides in this case) invalidates the calculations. */
    } while(updateTriesCount_ < maxUpdateTries_ && updateSlidersWithOverflow(overflow));

    calculator.calculateElementsOffsetDueToScroll(this, {
        hSlider_ ? hSlider_->getScrollValue() : 0,
        vSlider_ ? vSlider_->getScrollValue() : 0});

    layoutNext();
}

auto UIPane::event(state::UIStatePtr& state) -> void
{
    /* Let the base do the generic stuff like mouse move pre-pass. */
    UIBase::event(state);

    updateClosestSlider(state);

    const auto eId = state->currentEventId;
    if (eId == MouseButtonEvt::eventId && state->hoveredId == id_)
    {
        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        MouseButtonEvt e{state->mouseButton, state->mouseAction};
        return emitEvent<MouseButtonEvt>(e);
    }

    eventNext(state);
}

auto UIPane::updateSlidersWithOverflow(const glm::vec2& overflow) -> bool
{
    bool needsRecalc{false};
    if (hSlider_ && overflow.x > 0)
    {
        if (!hSlider_->isParented()) { add(hSlider_); needsRecalc = true; }
        hSlider_->setScrollTo(overflow.x);
    }
    else if (hSlider_ && overflow.x <= 0 && hSlider_->isParented())
    {
        hSlider_->setScrollValue(0);
        if (hSlider_->isParented()) { remove(hSlider_); needsRecalc = true; }
    }

    if (vSlider_ && overflow.y > 0)
    {
        if (!vSlider_->isParented()) { add(vSlider_); needsRecalc = true; }
        vSlider_->setScrollTo(overflow.y);
    }
    else if (vSlider_ && overflow.y <= 0 && vSlider_->isParented())
    {
        vSlider_->setScrollValue(0);
        if (vSlider_->isParented()) { remove(vSlider_); needsRecalc = true; }
    }

    return needsRecalc;
}

auto UIPane::updateClosestSlider(state::UIStatePtr& state) -> void
{
    /* Get the closest scrollbar available in this pane and prioritize the verical direction. Mouse needs
        to be inside the pane. If the mouse is not on the slider, assume closest is the vertical one, if
        available, otherwise the horizontal one. If the mouse is inside one of the sliders, that's the
        closest one. */
    if (state->currentEventId != MouseMoveScanEvt::eventId) {return; }
    if (state->hoveredId != id_) {return; }
    if (!isPointInsideView(state->mousePos)) {return; }

    //TODO: Needs more work: if the last pane only has hSlider, the previous vSlider will be overwritten
    if (hSlider_ && hSlider_->isParented())
    {
        state->closestScroll = hSlider_->getId();
    }

    if (vSlider_ && vSlider_->isParented())
    {
        state->closestScroll = vSlider_->getId();
    }

    if (vSlider_ && vSlider_->isParented() && vSlider_->isPointInsideView(state->mousePos))
    {
        state->closestScroll = vSlider_->getId();
    }
    else if (hSlider_ && hSlider_->isParented() && hSlider_->isPointInsideView(state->mousePos))
    {
        state->closestScroll = hSlider_->getId();
    }
}

auto UIPane::setScrollEnabled(const bool enableH, const bool enableV) -> UIPane&
{
    if (enableV)
    {
        vSlider_ = utils::make<UISlider>();
        vSlider_->setInvertAxis(true);
        vSlider_->setCustomTagId(UISlider::scrollTagId);
        vSlider_->setType(LayoutBase::Type::VERTICAL)
            .setScale({20_px, 1.0_rel});
    }
    else { vSlider_.reset(); }

    if (enableH)
    {
        hSlider_ = utils::make<UISlider>();
        hSlider_->setCustomTagId(UISlider::scrollTagId);
        hSlider_->setType(LayoutBase::Type::HORIZONTAL)
            .setScale({1.0_rel, 20_px});
    }
    else { hSlider_.reset(); }

    return *this;
}

auto UIPane::setScrollSensitivityMultiplier(const float value) -> UIPane&
{
    vSlider_ ? vSlider_->setScrollSensitivity(value) : void();
    hSlider_ ? hSlider_->setScrollSensitivity(value) : void();
    return *this;
}

auto UIPane::isVerticalOverflow() const -> bool { return vSlider_ ? true : false; }

auto UIPane::isHorizontalOverflow() const -> bool { return hSlider_ ? true : false; }

auto UIPane::getHorizontalSlider() const -> UISliderWPtr { return hSlider_; }

auto UIPane::getVerticalSlider() const -> UISliderWPtr { return vSlider_; }
} // namespace src::uielements
