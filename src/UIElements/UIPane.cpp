#include "UIPane.hpp"

#include "src/ElementComposable/LayoutAttribs.hpp"
#include "src/ElementComposable/IEvent.hpp"
#include "src/LayoutCalculator/BasicCalculator.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/Utils/Misc.hpp"
#include "vendor/glfw/include/GLFW/glfw3.h"

namespace src::uielements
{
UIPane::UIPane()
{
    //TODO: These shall be created on demand and only if the layout permits
    vSlider_ = utils::make<UISlider>();
    vSlider_->setColor(utils::hexToVec4("#ffffffff"));
    vSlider_->enableVerticalInversion(true);
    vSlider_->setCustomTagId(1000);
    vSlider_->setLayoutType(LayoutAttribs::Type::VERTICAL)
        .setLayoutScale({20_px, 1.0_rel})
        .setLayoutEnableCustomIndex(true)
        .setLayoutIndex(4);

    hSlider_ = utils::make<UISlider>();
    hSlider_->setColor(utils::hexToVec4("#ffffffff"));
    hSlider_->setCustomTagId(1000);
    hSlider_->setLayoutType(LayoutAttribs::Type::HORIZONTAL)
        .setLayoutScale({1.0_rel, 20_px})
        .setLayoutEnableCustomIndex(true)
        .setLayoutIndex(4);
}

auto UIPane::render(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", getLayoutTransform());
    shader_.uploadVec4f("uColor", color_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    static auto l = [](const UIBasePtr& e) { return e->getCustomTagId () == 1000; };
    renderNextExcept(projection, l);
    renderNextSingle(projection, vSlider_);
    renderNextSingle(projection, hSlider_);
}

auto UIPane::layout() -> void
{
    using namespace layoutcalculator;
    const auto& calculator = BasicCalculator::get();

    //TODO: Cap the while to max one or two retries. Anything else is too much.
    while (true)
    {
        const auto sliderImpact = calculator.calcPaneSliders(this);
        calculator.calcPaneElements(this, sliderImpact);
    
        const auto overflow = calculator.calcOverflow(this, sliderImpact);

        /* Adding new elements (slides in this case) invalidates the calculations. */
        if (const auto needsRecalc = updateSlidersWithOverflow(overflow); !needsRecalc) { break; }
    }

    calculator.calcPaneElementsAddScrollToPos(this, {
        hSlider_ ? hSlider_->getScrollValue() : 0,
        vSlider_ ? vSlider_->getScrollValue() : 0});

    layoutNext();
}

auto UIPane::event(framestate::FrameStatePtr& state) -> void
{
    /* Let the base do the generic stuff like mouse move pre-pass. */
    UIBase::event(state);

    const auto eId = state->currentEventId;
    /* Get the closes scrollbar available in this pane and prioritize the verical direction. Mouse needs
        to be inside the pane. If the mouse is not on the slider, assume closest is the vertical one, if
        available, otherwise the horizontal one. If the mouse is inside one of the sliders, that's the closest one.
    */
    if (eId == MouseMoveScanEvt::eventId && state->hoveredId == id_ && isPointInsideView(state->mousePos))
    {
        state->closestScroll = vSlider_->isParented() ? vSlider_->getId() : hSlider_->getId();
        if (vSlider_->isParented() && vSlider_->isPointInsideView(state->mousePos))
        {
            state->closestScroll = vSlider_->getId();
        }
        else if (hSlider_->isParented() && hSlider_->isPointInsideView(state->mousePos))
        {
            state->closestScroll = hSlider_->getId();
        }
    }
    else if (eId == MouseButtonEvt::eventId && state->hoveredId == id_)
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
    if (overflow.x > 0)
    {
        if (!hSlider_->isParented()) { add(hSlider_); needsRecalc = true; }
        hSlider_->setScrollTo(std::clamp(overflow.x, 0.0f, 9999.0f));
    }
    else if (overflow.x <= 0 && hSlider_->isParented())
    {
        hSlider_->setScrollValue(0);
        if (hSlider_->isParented()) { remove(hSlider_); needsRecalc = true; }
    }

    if (overflow.y > 0)
    {
        if (!vSlider_->isParented()) { add(vSlider_); needsRecalc = true; }
        vSlider_->setScrollTo(std::clamp(overflow.y, 0.0f, 9999.0f));
    }
    else if (overflow.y <= 0 && vSlider_->isParented())
    {
        vSlider_->setScrollValue(0);
        if (vSlider_->isParented()) { remove(vSlider_); needsRecalc = true; }
    }

    return needsRecalc;
}

auto UIPane::isVerticalOverflow() const -> bool { return vSlider_ ? true : false; }

auto UIPane::isHorizontalOverflow() const -> bool { return hSlider_ ? true : false; }

auto UIPane::getHorizontalSlider() const -> UISliderWPtr { return hSlider_; }

auto UIPane::getVerticalSlider() const -> UISliderWPtr { return vSlider_; }
} // namespace src::uielements
