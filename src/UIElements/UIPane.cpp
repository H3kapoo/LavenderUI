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
    vSlider_ = utils::make<UISlider>();
    vSlider_->setColor(utils::hexToVec4("#ffffffff"));
    vSlider_->enableVerticalInversion(true);
    vSlider_->setCustomTagId(1000);
    vSlider_->setLayoutType(LayoutAttribs::Type::VERTICAL)
        .setLayoutScale({40_px, 1.0_rel})
        .setLayoutEnableCustomIndex(true)
        .setLayoutIndex(4);

    // hSlider_ = utils::make<UISlider>();
    // hSlider_->setColor(utils::hexToVec4("#ffffffff"));
    // hSlider_->setCustomTagId(1000);
    // hSlider_->setLayoutType(LayoutAttribs::Type::HORIZONTAL)
    //     .setLayoutScale({1.0_rel, 40_px})
    //     .setLayoutEnableCustomIndex(true)
    //     .setLayoutIndex(4);

    // add(vSlider_);
    // add(hSlider_);
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
    const auto sliderImpact = calculator.calcPaneSliders(this);
    calculator.calcPaneElements(this, sliderImpact);

    const auto overflow = calculator.calcOverflow(this, sliderImpact);
    // log_.debug("overflow {} {}", overflow.x, overflow.y);
    if (overflow.y > 0)
    {
        // log_.error("added");
        if (!vSlider_->isParented()) { add(vSlider_); }
        vSlider_->setScrollTo(std::clamp(overflow.y, 0.0f, 9999.0f));
    }
    else if (overflow.y <= 0 && vSlider_->isParented())
    {
        // log_.error("removed");
        vSlider_->setScrollValue(0);
        if (vSlider_->isParented()) { remove(vSlider_); }
    }
    // hSlider_->setScrollTo(std::clamp(overflow.x, 0.0f, 9999.0f));

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
    if (eId == MouseButtonEvt::eventId && state->hoveredId == id_)
    {
        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        MouseButtonEvt e{state->mouseButton, state->mouseAction};
        return emitEvent<MouseButtonEvt>(e);
    }

    eventNext(state);
}

auto UIPane::isVerticalOverflow() const -> bool { return vSlider_ ? true : false; }

auto UIPane::isHorizontalOverflow() const -> bool { return hSlider_ ? true : false; }

auto UIPane::getHorizontalSlider() const -> UISliderWPtr { return hSlider_; }

auto UIPane::getVerticalSlider() const -> UISliderWPtr { return vSlider_; }
} // namespace src::uielements
