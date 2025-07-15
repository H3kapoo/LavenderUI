#include "UITreeView.hpp"

#include "src/ElementComposable/LayoutAttribs.hpp"
#include "src/ElementComposable/IEvent.hpp"
#include "src/LayoutCalculator/BasicCalculator.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/Utils/Misc.hpp"
#include "vendor/glfw/include/GLFW/glfw3.h"

namespace src::uielements
{
UITreeView::UITreeView()
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

    setLayoutType(LayoutAttribs::Type::VERTICAL);
}

auto UITreeView::render(const glm::mat4& projection) -> void
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

auto UITreeView::layout() -> void
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

auto UITreeView::event(framestate::FrameStatePtr& state) -> void
{
    /* Let the base do the generic stuff like mouse move pre-pass. */
    UIBase::event(state);

    const auto eId = state->currentEventId;

    eventNext(state);
}
} // namespace src::uielements
