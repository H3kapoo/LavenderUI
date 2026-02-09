#include <LavenderUI/Node/UISlider.hpp>

#include <LavenderUI/Core/EventHandler/CoreEvents/MouseDrag.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseScroll.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseEnter.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseExit.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseLeftClick.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/Slide.hpp>
#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/LayoutHandler/Calculators/SliderCalculator.hpp>
#include <LavenderUI/Node/UILabel.hpp>
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::node
{
UISlider::UISlider(UIBaseInitData&& initData)
    : UIBase(std::move(initData))
    , knobLayout_()
    , label_(utils::make<UILabel>())
    , knobColor_(utils::hexToVec4("#afafafff"))
    , scrollFrom_(0.0f)
    , scrollTo_(100.0f)
    , scrollValue_(0.0f)
    , percentage_(0.0f)
    , distToKnobCenter_(0.0f)
    , sensitivity_(5.0f)
    , invertVertical_(false)
{
    knobColor_ = utils::hexToVec4("#ca5555ff");
    label_->setColor(utils::hexToVec4("#ffffff00"));
    label_->getBaseLayoutData().setScale({1_fill, 1_fill});

    setScrollFrom(0);
    layoutBase_.setMargin(5);

    UIBase::add(label_);
}

auto UISlider::onRender(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", baseColor_);
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    shader_.uploadVec4f("uBorderRadii", layoutBase_.getBorderRadius());
    shader_.uploadVec4f("uBorderColor", borderColor_);
    shader_.uploadInt("uUseTexture", 0);
    core::GPUBinder::get().renderBoundQuad();

    /* Draw knob */
    core::GPUBinder::get().enable(core::GPUBinder::Function::DEPTH, false);
    shader_.bind();
    shader_.uploadMat4("uMatrixTransform", knobLayout_.getTransform());
    shader_.uploadVec4f("uColor", knobColor_);
    shader_.uploadVec2f("uResolution", knobLayout_.getComputedScale());
    shader_.uploadVec4f("uBorderRadii", knobLayout_.getBorderRadius());

    core::GPUBinder::get().renderBoundQuad();
    core::GPUBinder::get().enable(core::GPUBinder::Function::DEPTH, true);
}

auto UISlider::onLayout() -> void
{
    const auto& calculator = core::SliderCalculator::get();
    calculator.calculateKnobScale(this, knobLayout_, scrollTo_);
    calculator.calculateKnobPosition(this, knobLayout_, percentage_, invertVertical_);

    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForGenericElement(this);
}

auto UISlider::onEvent(core::UIStatePtr& state) -> void
{
    const auto eId = state->currentEventId;
    if (eId == core::MouseScrollEvt::eventId)
    {
        // NOTE: inverting affects horizontal sliders. No side effects really.
        setScrollValue(scrollValue_ + state->scrollOffset.y * sensitivity_ * (invertVertical_ ? -1 : 1));

        core::SlideEvt slideEvt{getScrollValue()}; 
        eventsMgr_.emitEvent<core::SlideEvt>(slideEvt);
    }
    else if (eId == core::MouseDragEvt::eventId)
    {
        percentage_ = calculatePercentage(state->mousePos - distToKnobCenter_);

        core::SlideEvt slideEvt{getScrollValue()};
        eventsMgr_.emitEvent<core::SlideEvt>(slideEvt);

        core::MouseDragEvt e;
        eventsMgr_.emitEvent<core::MouseDragEvt>(e);
    }
    else if (eId == core::MouseLeftClickEvt::eventId)
    {
        const glm::vec2 knobHalf = knobLayout_.getComputedScale() / 2.0f;
        const glm::ivec2 middle = knobLayout_.getComputedPos() + knobHalf;
        distToKnobCenter_ = state->mousePos - middle;
        distToKnobCenter_ = utils::valueIfLowerAbs(distToKnobCenter_, knobHalf);
        percentage_ = calculatePercentage(state->mousePos - distToKnobCenter_);

        core::SlideEvt sliderEvt{getScrollValue()};
        eventsMgr_.emitEvent<core::SlideEvt>(sliderEvt);
    }
    else if (eId == core::MouseEnterEvt::eventId)
    {
        core::MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseEnterEvt>(e);
    }
    else if (eId == core::MouseExitEvt::eventId)
    {
        core::MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseExitEvt>(e);
    }

    setText(std::to_string((int)scrollValue_));
}

auto UISlider::calculatePercentage(const glm::ivec2& mPos) -> double
{
    const auto& computedPos = layoutBase_.getComputedPos();
    const auto& computedScale = layoutBase_.getComputedScale();
    const glm::ivec2 halfKnobScale = knobLayout_.getComputedScale() / 2.0f;
    if (layoutBase_.isHorizontal())
    {
        return utils::remap(mPos.x,
            computedPos.x + halfKnobScale.x, computedPos.x + computedScale.x - halfKnobScale.x,
            0.0f, 1.0f);
    }
    else if (layoutBase_.isVertical())
    {
        const double remapped = utils::remap(mPos.y,
            computedPos.y + halfKnobScale.y, computedPos.y + computedScale.y - halfKnobScale.y,
            0.0f, 1.0f);
        return invertVertical_ ? remapped : 1.0f - remapped;
    }

    return 0.0f;
}

auto UISlider::getKnobBaseLayoutData() -> core::LayoutBase&
{
    return knobLayout_;
}

auto UISlider::getLabel() -> UILabelWPtr
{
    return label_;
}

auto UISlider::getScrollPercentage() -> double
{
    return percentage_;
}

auto UISlider::getScrollValue() -> double
{
    scrollValue_ = utils::remap(percentage_, 0.0f, 1.0f, scrollFrom_, scrollTo_);
    return scrollValue_;
}

auto UISlider::setScrollValue(const double value) -> void
{
    percentage_ = utils::remap(value, scrollFrom_, scrollTo_, 0.0f, 1.0f);
}

auto UISlider::setScrollFrom(const double value) -> void
{
    scrollFrom_ = value;
    setText(std::to_string((int)getScrollValue()));
}

auto UISlider::setScrollTo(const double value) -> void
{
    scrollTo_ = value;
    setText(std::to_string((int)getScrollValue()));
}

auto UISlider::setScrollSensitivity(const float value) -> void
{
    sensitivity_ = value;
}

auto UISlider::setKnobColor(const glm::vec4& value) -> void
{
    knobColor_ = value;
}

auto UISlider::setText(const std::string& text) -> void { 
    // Disabled until we figure out more about how to deal with text
    label_->setText(text);
}

auto UISlider::setInvertAxis(const bool value) -> void
{
    invertVertical_ = value;
}
} // namespace lav::node
