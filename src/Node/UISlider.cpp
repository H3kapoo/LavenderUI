#include "UISlider.hpp"

#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Node/UILabel.hpp"
#include "src/Utils/Misc.hpp"
#include "src/Core/LayoutHandler/BasicCalculator.hpp"

namespace lav::node
{
UISlider::UISlider(UIBaseInitData&& initData) : UIBase(std::move(initData))
{
    using namespace core; // TODO: _px needs to be exposed to the lav namespace instead of ::core
    knobColor_ = utils::hexToVec4("#ca5555ff");
    label_->setColor(utils::hexToVec4("#ffffff00"));
    label_->getBaseLayoutData().setScale({1_fill, 1_fill});

    setScrollFrom(0);

    UIBase::add(label_);
}

auto UISlider::render(const glm::mat4& projection) -> void
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
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", knobLayout_.getTransform());
    shader_.uploadVec4f("uColor", knobColor_);
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    core::GPUBinder::get().renderBoundQuad();
    core::GPUBinder::get().enable(core::GPUBinder::Function::DEPTH, true);
}

auto UISlider::layout() -> void
{
    const auto& computedScale = layoutBase_.getComputedScale();
    if (layoutBase_.isHorizontal())
    {
        knobLayout_.setComputedScale({
            std::max(computedScale.y, computedScale.x - scrollTo_), computedScale.y});
    }
    else if (layoutBase_.isVertical())
    {
        knobLayout_.setComputedScale({computedScale.x,
            std::max(computedScale.x, computedScale.y - scrollTo_)});
    }

    calculateKnobPosition();

    const auto& calculator = core::BasicCalculator::get();
    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForGenericElement(this);
}

auto UISlider::event(node::UIStatePtr& state) -> void
{
    using namespace core;

    const auto eId = state->currentEventId;
    if (eId == MouseScrollEvt::eventId)
    {
        // NOTE: inverting affects horizontal sliders. No side effects really.
        setScrollValue(scrollValue_ + state->scrollOffset.y * sensitivity_ * (invertVertical_ ? -1 : 1));

        SliderEvt sliderEvt{getScrollValue()}; 
        eventsMgr_.emitEvent<SliderEvt>(sliderEvt);
    }
    else if (eId == MouseDragEvt::eventId)
    {
        percentage_ = calculatePercentage(state->mousePos - distToKnobCenter_);

        SliderEvt sliderEvt{getScrollValue()};
        eventsMgr_.emitEvent<SliderEvt>(sliderEvt);

        MouseDragEvt e;
        eventsMgr_.emitEvent<MouseDragEvt>(e);
    }
    else if (eId == MouseLeftClickEvt::eventId)
    {
        const glm::vec2 knobHalf = knobLayout_.getComputedScale() / 2.0f;
        const glm::ivec2 middle = knobLayout_.getComputedPos() + knobHalf;
        distToKnobCenter_ = state->mousePos - middle;
        distToKnobCenter_ = utils::valueIfLowerAbs(distToKnobCenter_, knobHalf);
        percentage_ = calculatePercentage(state->mousePos - distToKnobCenter_);

        SliderEvt sliderEvt{getScrollValue()};
        eventsMgr_.emitEvent<SliderEvt>(sliderEvt);
    }
    else if (eId == MouseEnterEvt::eventId)
    {
        MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<MouseEnterEvt>(e);
    }
    else if (eId == MouseExitEvt::eventId)
    {
        MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<MouseExitEvt>(e);
    }

    setText(std::to_string((int)scrollValue_));
}

auto UISlider::calculatePercentage(const glm::ivec2& mPos) -> float
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
        const float remapped = utils::remap(mPos.y,
            computedPos.y + halfKnobScale.y, computedPos.y + computedScale.y - halfKnobScale.y,
            0.0f, 1.0f);
        return invertVertical_ ? remapped : 1.0f - remapped;
    }

    return 0.0f;
}

auto UISlider::calculateKnobPosition() -> void
{
    const auto& computedPos = layoutBase_.getComputedPos();
    const auto& computedScale = layoutBase_.getComputedScale();
    if (layoutBase_.isHorizontal())
    {
        knobLayout_.setComputedPos({
            utils::remap(percentage_, 0.0f, 1.0f,
                computedPos.x, computedPos.x + computedScale.x - knobLayout_.getComputedScale().x),
                computedPos.y});
    }
    else if (layoutBase_.isVertical())
    {
        knobLayout_.setComputedPos({computedPos.x,
            utils::remap(invertVertical_ ? percentage_ : 1.0f - percentage_, 0.0f, 1.0f,
                computedPos.y, computedPos.y + computedScale.y - knobLayout_.getComputedScale().y)});
    }
}

auto UISlider::getKnobBaseLayoutData() -> core::LayoutBase& { return knobLayout_; }

auto UISlider::getLabel() -> UILabelWPtr { return label_; }

auto UISlider::getScrollPercentage() -> float { return percentage_; }

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
    setText(std::to_string((int)getScrollValue()));
}

auto UISlider::setScrollTo(const float value) -> void
{
    scrollTo_ = value;
    setText(std::to_string((int)getScrollValue()));
}

auto UISlider::setScrollSensitivity(const float value) -> void { sensitivity_ = value; }

auto UISlider::setText(const std::string& text) -> void { label_->setText(text); }

auto UISlider::setInvertAxis(const bool value) -> void { invertVertical_ = value; }
} // namespace lav::node
