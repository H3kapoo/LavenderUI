#include "PaneCalculator.hpp"

namespace lav::core
{
auto PaneCalculator::get() -> PaneCalculator&
{
    static PaneCalculator instance;
    return instance;
}

auto PaneCalculator::calculateElementsOffsetDueToScroll(node::UIPane* parent,
    const glm::ivec2 offset) const -> void
{
    const auto& elements = parent->getElements();
    for (const auto& element : elements)
    {
        SKIP_SLIDER(element);
        auto& eLayout = element->getBaseLayoutData();
        eLayout.setComputedPos(eLayout.getComputedPos() - offset);
    }
}

auto PaneCalculator::calculateSlidersScaleAndPos(node::UIPane* parent) const -> glm::vec2
{
    glm::vec2 sliderImpact{0, 0};
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pComputedPos = pLayout.getComputedPos();
    const auto& pComputedScale = pLayout.getComputedScale();
    if (const auto vSlider = parent->getVerticalScroll().lock(); vSlider && vSlider->isParented())
    {
        // Scroll sliders on a Pane can ONLY have PX values on the scroll direction.
        auto& vLayout = vSlider->getBaseLayoutData();
        sliderImpact.x = vLayout.getScale().x.val;
        vLayout.setComputedPos({
            pComputedPos.x + pComputedScale.x - sliderImpact.x,
            pComputedPos.y
        });
        vLayout.setComputedScale({sliderImpact.x, pComputedScale.y});
    }

    if (const auto hSlider = parent->getHorizontalScroll().lock(); hSlider && hSlider->isParented())
    {
        // Scroll sliders on a Pane can ONLY have PX values on the scroll direction.
        auto& hLayout = hSlider->getBaseLayoutData();
        sliderImpact.y = hLayout.getScale().y.val;
        hLayout.setComputedPos({
            pComputedPos.x,
            pComputedPos.y + pComputedScale.y - sliderImpact.y,
        });
        hLayout.setComputedScale({pComputedScale.x - sliderImpact.x, sliderImpact.y});
    }

    return sliderImpact;
}
} // namespace lav::core