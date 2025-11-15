#include "UIPane.hpp"

#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LayoutHandler/BasicCalculator.hpp"
#include "src/Core/ResourceHandler/Shader.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::node
{
UIPane::UIPane(UIBaseInitData&& initData) : UIBase(std::move(initData))
{
    using namespace core;
    layoutBase_.setScale({200_px, 50_px});
}

auto UIPane::render(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", baseColor_);
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    shader_.uploadVec4f("uBorderColor", borderColor_);
    shader_.uploadInt("uUseTexture", 0);
    core::GPUBinder::get().renderBoundQuad();
}

auto UIPane::layout() -> void
{
    const auto& calculator = core::BasicCalculator::get();

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
    // } while(updateTriesCount_ < maxUpdateTries_ && updateSlidersWithOverflow(overflow));
    } while(updateSlidersWithOverflow(overflow) && updateTriesCount_ < maxUpdateTries_);

    calculator.calculateElementsOffsetDueToScroll(this, {
        hScroll_ ? hScroll_->getScrollValue() : 0,
        vScroll_ ? vScroll_->getScrollValue() : 0});
}

auto UIPane::event(node::UIStatePtr& state) -> void
{
    using namespace core;

    updateClosestSlider(state);

    const auto eId = state->currentEventId;
    if (eId == MouseButtonEvt::eventId && state->hoveredId == id_)
    {
        MouseButtonEvt e{state->mouseButton, state->mouseAction};
        eventsMgr_.emitEvent<MouseButtonEvt>(e);
    }
}

auto UIPane::updateSlidersWithOverflow(const glm::vec2& overflow) -> bool
{
    bool needsRecalc{false};
    if (hScroll_ && overflow.x > 0)
    {
        if (!hScroll_->isParented()) { add(hScroll_); needsRecalc = true; }
        hScroll_->setScrollTo(overflow.x);
    }
    else if (hScroll_ && overflow.x <= 0 && hScroll_->isParented())
    {
        hScroll_->setScrollValue(0);
        if (hScroll_->isParented()) { remove(hScroll_); needsRecalc = true; }
    }

    if (vScroll_ && overflow.y > 0)
    {
        if (!vScroll_->isParented()) { add(vScroll_); needsRecalc = true; }
        vScroll_->setScrollTo(overflow.y);
    }
    else if (vScroll_ && overflow.y <= 0 && vScroll_->isParented())
    {
        vScroll_->setScrollValue(0);
        if (vScroll_->isParented()) { remove(vScroll_); needsRecalc = true; }
    }

    return needsRecalc;
}

auto UIPane::updateClosestSlider(node::UIStatePtr& state) -> void
{
    /* Get the closest scrollbar available in this pane and prioritize the verical direction. Mouse needs
        to be inside the pane. If the mouse is not on the slider, assume closest is the vertical one, if
        available, otherwise the horizontal one. If the mouse is inside one of the sliders, that's the
        closest one. */
    if (state->currentEventId != core::MouseMoveScanEvt::eventId) {return; }
    if (state->hoveredId != id_) {return; }
    if (!layoutBase_.isPointInsideView(state->mousePos)) {return; }

    //TODO: Needs more work: if the last pane only has hSlider, the previous vSlider will be overwritten
    if (hScroll_ && hScroll_->isParented())
    {
        state->closestScrollId = hScroll_->getId();
    }

    if (vScroll_ && vScroll_->isParented())
    {
        state->closestScrollId = vScroll_->getId();
    }

    if (vScroll_ && vScroll_->isParented() && vScroll_->getBaseLayoutData().isPointInsideView(state->mousePos))
    {
        state->closestScrollId = vScroll_->getId();
    }
    else if (hScroll_ && hScroll_->isParented() && hScroll_->getBaseLayoutData().isPointInsideView(state->mousePos))
    {
        state->closestScrollId = hScroll_->getId();
    }
}

auto UIPane::setScrollEnabled(const bool enableH, const bool enableV) -> UIPane&
{
    using namespace core;
    if (enableV)
    {
        vScroll_ = utils::make<UIScroll>();
        vScroll_->setInvertAxis(true);
        vScroll_->getBaseLayoutData().setType(LayoutBase::Type::VERTICAL)
            .setScale({20_px, 1.0_rel});
    }
    else { vScroll_.reset(); }

    if (enableH)
    {
        hScroll_ = utils::make<UIScroll>();
        // hScroll_->setColor(utils::hexToVec4("#aaaaaaff"));
        hScroll_->getBaseLayoutData().setType(LayoutBase::Type::HORIZONTAL)
            .setScale({1.0_rel, 20_px});
    }
    else { hScroll_.reset(); }

    return *this;
}

auto UIPane::setScrollSensitivityMultiplier(const float value) -> UIPane&
{
    vScroll_ ? vScroll_->setScrollSensitivity(value) : void();
    hScroll_ ? hScroll_->setScrollSensitivity(value) : void();
    return *this;
}

auto UIPane::isVerticalOverflow() const -> bool { return vScroll_ ? true : false; }

auto UIPane::isHorizontalOverflow() const -> bool { return hScroll_ ? true : false; }

auto UIPane::getHorizontalSlider() const -> UISliderWPtr { return hScroll_; }

auto UIPane::getVerticalSlider() const -> UISliderWPtr { return vScroll_; }
} // namespace lav::node
