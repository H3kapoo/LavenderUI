#pragma once

#include "include/LavenderUI/Core/LayoutHandler/BaseCalculator.hpp"
#include "include/LavenderUI/Node/UISplitPane.hpp"
#include "include/LavenderUI/Utils/Logger.hpp"

namespace lav::core
{
class SplitPaneCalculator : public BaseCalculator
{
public:
    static auto get() -> SplitPaneCalculator&;

    /** @brief Calculates the `computedPos` and `computedScale` of a UISPlitPane element.

        @details Function calculates position and scale according to `setScale` relative values
            provided by the user and also take into account `minScale` and `maxScale` of each element.
            This will try to resize elements to fulfill min/max and scale as best as possible.
        @details Also if there's a handleIdx value set, relative scale values will be updated reflecting
            user's left/right/top/bottom dragging of the handle controlled panes.

        @param parent Element for which split pane elements need to be calculated
        @param handleIdx Currently active split pane handle
        @param mousePos Current mouse position
    */
    auto calculateSplitPaneElements(node::UISplitPane* parent, const uint32_t handleIdx,
        const glm::vec2 mousePos) const -> void;

private:
    SplitPaneCalculator();
    SplitPaneCalculator(const SplitPaneCalculator&) = delete;
    SplitPaneCalculator(SplitPaneCalculator&&) = delete;
    auto operator=(const SplitPaneCalculator&) -> SplitPaneCalculator& = delete;
    auto operator=(SplitPaneCalculator&&) -> SplitPaneCalculator& = delete;

    /** @brief Simply calculate the scale of the handles of this split pane and return their accumulated size.

        @param parent SplitPane element for which to calculate handle sizes.

        @return Accumulated size of each handle.
    */
    auto calculateHandlesScale(node::UISplitPane* parent) const -> glm::ivec2;


    /** @brief Simply calculate the scale of the NON-handle elements of this split pane.

        @param parent SplitPane element for which to calculate NON-handle element sizes.
    */
    auto calculateNonHandlElementsScale(node::UISplitPane* parent,
        const glm::vec2 handlesSize) const -> void;


    /** @brief Try calculate and apply scale corrections for the panes if needed.

        @details When resizing the split pane area we might end up with a pane going under or over
            the minimum, respectively maximum scale set. This will try to keep those constrains
            satisfied as best as possible.

        @param parent SplitPane element for which to apply corrections.
        @param handlesSize Previously calculated accumulated handles size.
    */
    auto calculateElementsScaleCorrection(node::UISplitPane* parent,
        const glm::vec2 handlesSize) const -> void;


    /** @brief Simply calculate the position of all the elements of the split pane.

        @param parent SplitPane element for which to calculate positions.
    */
    auto calculateElementsPos(node::UISplitPane* parent) const -> void;


    /** @brief Adjust the `handleIdx` controlled panes relative scale by the amount the user has dragged.

        @param parent SplitPane element for which to calculate positions.
        @param handleIdx Currently under drag pane handle
        @param handlesSize Total size of all the handles in the split pane
        @param mousePos Current mouse position
    */
    auto calculateRelativeValuesDueToDrag(node::UISplitPane* parent,
        const uint32_t handleIdx, const glm::vec2 handlesSize, const glm::vec2 mousePos) const -> void;

    /** @brief Helper function that constrains the wanted offset between min and max of each pane

        @note Here lp* and rp* basically refer to the first and second handle controlled pane.

        @param wantedOffset Wanted offset due to drag or resize
        @param lpScale First pane current relative scale
        @param lpMin First pane minimum set scale
        @param lpMax First pane maximum set scale
        @param rpScale Second pane current relative scale
        @param rpMin Second pane minimum set scale
        @param rpMax Second pane maximum set scale

        @return Capped offset between the mins and maxes
    */
    auto constrainOffset(float wantedOffset,
        const float lpScale, const float lpMin, const float lpMax,
        const float rpScale, const float rpMin, const float rpMax) const -> float;


private:
    utils::Logger log_;
};
} // namespace lav::core