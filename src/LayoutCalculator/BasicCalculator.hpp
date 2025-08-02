#pragma once

#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UISplitPane.hpp"

namespace src::layoutcalculator
{
/**
    @brief Simple layout calculator for scaling and placing GUI elements in their appropriate place.
        Can be used as a template to derive other layout mechanics.

    @note Singleton class as each layout pass shall be stateless for a better view of the layout pipeline.
    @note There is a limitation on using shared_from_this() in the derived classes that inherit UIBase.
            Since UIBase inherits enable_shared_from_this<UIBase>, the shared_from_this() returned when used inside
            a derived class is of type UIBase and it will corrupt the shared/weak ptr control block when trying to do
            sharedParent->getSomethingShared/Weak inside the layout calculator (derived_func(shared_from_this()).
            The workaround for this is to use raw pointers and call the functions with "this" instead of
            "shared_from_this". It is safe anyway as the class doesnt save anything or delete the pointers.

    @note   Ex of explained:
            in calculator:
            castedParent->getSomeInternalUIElementShared() => this will crash
                                                            if it was called from derived class with
                                                            parent = shared_from_this
*/
class BasicCalculator
{
public:
    static auto get() -> BasicCalculator&;

    /** @brief Calculate the computed scale for the elements of this parent element & get the total scale.

        @details Function calculates the required `computedScale` based on the user supplied `scaleType` and
            `value` set via `setScale`.
        @details Parameters such as `margin`, `padding` and `border` are taken into consideration when
            calculating the `computedScale`.
        @details Refer to LayoutBase for more details.

        @param parent Element for which the subelements need to be calculated
        @param shrinkScaleBy Optional parameter to shrink the parent computed scale area if needed
            (usually used to make room for scroll bars)

        @return Total scale on each axis. The the elements of the parent element will now have the required computed scale.
    */
    auto calculateScaleForGenericElement(uielements::UIPane* parent,
        const glm::vec2 shrinkScaleBy = {}) const -> glm::vec2;

    /** @brief Calculate the computed position for the elements of this parent element.

        @details Function calculates the required `computedPos` based on layout rules such as `Type`, `wrap`, etc. or
            based on the user supplied `userPos` in case an absolute positioning is needed.
        @details Parameters such as `margins`, `padding`, etc. are taken into consideration when calculating the `computedPos`.
        @details Refer to LayoutBase for more details.

        @param parent Element for which the subelements need to be calculated
        @param maxScale Max scale on each axis for the calculated elements (as a unit)
        @param shrinkScaleBy Optional parameter to shrink the parent computed scale area if needed
            (usually used to make room for scroll bars)

        @return Void. But the elements of the parent element will now have the required computed pos.
    */
    auto calculatePositionForGenericElement(uielements::UIPane* parent, const glm::vec2 maxScale,
        const glm::vec2 shrinkScaleBy = {}) const -> void;

    /** @brief Adjust the previously calculated computedPos in order to obey user set parent alignment.

        @details Function adjusts `computedPos` based on user's parent `Align` rule.
        @details Refer to LayoutBase for more details.

        @param parent Element for which the subelements need to be adjusted
        @param overflow Previously calculated layout overflow

        @return Void. But the elements of the parent element will now have their `computedPos` ajusted as needed.
    */
    auto calculateAlignmentForElements(uielements::UIBase* parent, const glm::vec2 overflow) const -> void;

    auto calcPaneElements(uielements::UIPane* parent, const glm::vec2 scrollData) const -> void;
    auto calculateSlidersScaleAndPos(uielements::UIPane* parent) const -> glm::vec2;
    auto calcPaneElementsAddScrollToPos(uielements::UIPane* parent, const glm::ivec2 offset) const -> void;

    auto calcElementsPos(uielements::UIBase* parent, const glm::vec2 scrollData = {}) const -> void;
    auto calcElementsScale(uielements::UIBase* parent, const glm::vec2 scrollData = {}) const -> void;

    auto calcSplitPaneElements(uielements::UISplitPane* parent) const -> void;

    auto calcOverflow(uielements::UIBase* parent, const glm::vec2 shrinkScaleBy) const -> glm::vec2;

private:
    auto computeSpacingOnAxis(const elementcomposable::LayoutBase::Spacing spacing, const int32_t elementsCount,
        const float pContentScale, const float maxScale) const -> glm::vec2;

    auto computeFitScale(uielements::UIBase* parent) const -> glm::vec2;
};
} // namespace src::layoutcalculator
