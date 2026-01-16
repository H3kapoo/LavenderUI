#include "include/LavenderUI/Core/LayoutHandler/Calculators/SliderCalculator.hpp"

namespace lav::core
{
auto SliderCalculator::get() -> SliderCalculator&
{
    static SliderCalculator instance;
    return instance;
}

SliderCalculator::SliderCalculator()
    : log_(__func__)
{}

auto SliderCalculator::calculateKnobScale(node::UISlider* parent, LayoutBase& knobLayout,
    const float maxScroll) const -> void
{
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& computedScale = pLayout.getComputedScale();

    glm::vec2 knobComputedScale{0, 0};
    if (pLayout.isHorizontal())
    {
        knobComputedScale.x = std::max(computedScale.y, computedScale.x - maxScroll);
        knobComputedScale.y = computedScale.y;
    }
    else if (pLayout.isVertical())
    {
        knobComputedScale.x = computedScale.x;
        knobComputedScale.y = std::max(computedScale.x, computedScale.y - 1000);
    }

    // knobComputedScale -= glm::vec2{4};
    knobLayout.setComputedScale(knobComputedScale);
}

auto SliderCalculator::calculateKnobPosition(node::UISlider* parent, LayoutBase& knobLayout,
    const float scrollPercentage, const bool shouldInvertY) const -> void
{
    const auto& pLayout = parent->getBaseLayoutData();
    glm::vec2 pComputedPos = pLayout.getComputedPos();
    glm::vec2 pComputedScale = pLayout.getComputedScale();
    // computedPos += glm::vec2{2, 2};
    // computedScale -= glm::vec2{4, 4};

    const auto& knobComputedScale = knobLayout.getComputedScale();
    glm::vec2 knobComputedPos{0, 0};
    if (pLayout.isHorizontal())
    {
        knobComputedPos.x = utils::remap(
                scrollPercentage,
                0.0f,
                1.0f,
                pComputedPos.x,
                pComputedPos.x + pComputedScale.x - knobComputedScale.x);
        knobComputedPos.y = pComputedPos.y;
    }
    else if (pLayout.isVertical())
    {
        knobComputedPos.x = pComputedPos.x;
        knobComputedPos.y = utils::remap(
                shouldInvertY ? scrollPercentage : 1.0f - scrollPercentage,
                0.0f,
                1.0f,
                pComputedPos.y,
                pComputedPos.y + pComputedScale.y - knobLayout.getComputedScale().y); 
    }

    knobLayout.setComputedPos(knobComputedPos);
}
} // namespace lav::core