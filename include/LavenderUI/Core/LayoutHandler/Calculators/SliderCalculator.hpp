#pragma once

#include "include/LavenderUI/Core/LayoutHandler/BaseCalculator.hpp"
#include "include/LavenderUI/Core/LayoutHandler/LayoutBase.hpp"
#include "include/LavenderUI/Node/UISlider.hpp"
#include "include/LavenderUI/Utils/Logger.hpp"

namespace lav::core
{
class SliderCalculator : public BaseCalculator
{
public:
    static auto get() -> SliderCalculator&;

    auto calculateKnobScale(node::UISlider* parent, LayoutBase& knobLayout,
        const float maxScroll) const -> void;
    auto calculateKnobPosition(node::UISlider* parent, LayoutBase& knobLayout,
        const float scrollPercentage, const bool shouldInvertY) const -> void;

private:
    SliderCalculator();
    SliderCalculator(const SliderCalculator&) = delete;
    SliderCalculator(SliderCalculator&&) = delete;
    auto operator=(const SliderCalculator&) -> SliderCalculator& = delete;
    auto operator=(SliderCalculator&&) -> SliderCalculator& = delete;

    utils::Logger log_;
};
} // namespace lav::core