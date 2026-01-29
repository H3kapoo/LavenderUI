#pragma once

#include <LavenderUI/Core/LayoutHandler/BaseCalculator.hpp>
#include <LavenderUI/Utils/Logger.hpp>

namespace lav::core
{
class PaneCalculator : public BaseCalculator
{
public:
    static auto get() -> PaneCalculator&;

    /** @brief Calculates the new computed pos for the parent's elements by adding in the scroll offset.

        @param parent Element for which the elements need to be offset
        @param offset The offset by which to move the child element's position
    */
    auto calculateElementsOffsetDueToScroll(node::UIPane* parent, const glm::ivec2 offset) const -> void;

    /** @brief Calculate the computed pos and scale of all the slider scrollbars of a UIPane/UIPane derivate.

        @note As some other functions need to know what impact the sliders have on the parent layout before hand,
            it is best for this function to be ran as earliest as possible.
        @note Sliders that do no have the scrollbar role are not affected by this.

        @param parent Element for which the slider child elements need to be positioned and scaled

        @return How much space on each axis the slider(s) occupy.
    */
    auto calculateSlidersScaleAndPos(node::UIPane* parent) const -> glm::vec2;

private:
    PaneCalculator();
    PaneCalculator(const PaneCalculator&) = delete;
    PaneCalculator(PaneCalculator&&) = delete;
    auto operator=(const PaneCalculator&) -> PaneCalculator& = delete;
    auto operator=(PaneCalculator&&) -> PaneCalculator& = delete;

    utils::Logger log_;
};
} // namespace lav::core