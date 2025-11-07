#pragma once

// #include "src/Node/UIDropdown.hpp"
// #include "src/Node/UIPane.hpp"
// #include "src/Node/UISplitPane.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Utils/Logger.hpp"

namespace lav::core
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

    @note Ex of explained:
        In calculator:
        castedParent->getSomeInternalNodehared() => this will crash
                                                            if it was called from derived class with
                                                            parent = shared_from_this
*/
class BasicCalculator
{
public:
    static auto get() -> BasicCalculator&;

    /** @brief Calculate the computed scale for the elements of this parent element.

        @note This pass needs to be done before the positioning pass.
        @note Slider nodes with scrollbar role are ignored.

        @details Function calculates the required `computedScale` based on the user supplied `scaleType` and
            `value` set via `setScale`. Refer to LayoutBase for more details.
        @details Scaling can be of 4 types: PX, REL, FIT, FILL and FR on each axis. This will determine the computedScale
            of the element.
        @details PX  - Scale of the element expressed in plain pixel values (i.e 50_px)
        @details REL - Scale relative to the parent element and expressed in (0, 1] percentage range (i.e 0.4_rel). The parent
                  element scale is affected by: space taken by scroll bars (if any), padding and borders (of the parent).
                  Additionally, the computedScale of the element will be also affected by the element's margins.
        @details FIT - A Fit element will try to "gift-wrap" around it's OWN child elements such that they fit perfectly inside
                  of the parent. The fit space necessary is increased by the padding and borders of the element itself and by
                  any margins of it's OWN child elements.
                  Constraints for FIT element children on an arbitrary axis:
                    - Cannot be of type FILL/REL as that requires element computedScale to be already know but it's not yet.
                      We try to compute it in FIT.
                    - FIT elements can be FIT on their own but the leaf nodes MUST be of type PX, otherwise it's impossible
                      to find a size to fit around.
        @details FILL - Scale of the element will be an equal part (between it and other fill elements of parent) of what's left
                   after the PX/REL/FIT nodes aquire their computedScale.
                   A FILL element computedScale is affected (shrunk) by it's parent padding and border scale and also by it's
                   own margins.
        @details FR - Scale type specific only for GRID layouts. Grid cell will span exatly "fr" fractional parts. Fractional
                    parts are equal in size and are computed after subtracting all the PX grid cell sizes.
                    Example for column axis:
                        2_fr, 100_px, 1_fr
                    Second cell will occupy exactly 100PX on the X axis.
                    Total fractional parts of the column axis will be: 2_fr + 1_fr = 3_fr.
                    First cell will get 2 parts of the remaining space after subtracting the pixel occupied cells while
                    the third cell will get only 1 part.
                    Simply put the first cell will always be 2 times bigger than the third cell while the second cell remains
                    constant in size.
                    Child elements can be self aligned inside the cell(s) at the following positions via `selfAlign` setting.
                    Child elements can only be of scaleType PX, REL or FILL.

        @param parent Element for which the subelements need to be calculated
        @param shrinkScaleBy Optional parameter to shrink the parent computed scale area if needed
            (usually used to make room for scroll bars)
    */
    auto calculateScaleForGenericElement(node::UIBase* parent,
        const glm::vec2 shrinkScaleBy = {}) const -> void;


    /** @brief Calculate the computed position for the elements of this parent element.

        @note This pass needs to be done after the scaling pass.
        @note Slider nodes with scrollbar role are ignored.

        @details Function calculates the required `computedPos` based on layout rules such as `type`, `wrap`,
            `spacing`, `align` etc. or based on the user supplied `userPos` in case an absolute positioning is needed.
        @details Parameters such as `margins`, `padding`, etc. are taken into consideration when
            calculating the `computedPos`.
        @details Refer to LayoutBase for more details.

        @param parent Element for which the subelements need to be calculated
        @param shrinkScaleBy Optional parameter to shrink the parent computed scale area if needed
            (usually used to make room for scroll bars)

        @return Void. But the elements of the parent element will now have the required computed pos.
    */
    auto calculatePositionForGenericElement(node::UIBase* parent,
        const glm::vec2 shrinkScaleBy = {}) const -> void;


    /** @brief Adjust the previously calculated `computedPos` in order to obey user set parent alignment.

        @details Function adjusts `computedPos` based on user's parent `Align` rule.
        @details Refer to LayoutBase for more details.

        @param parent Element for which the subelements need to be adjusted
        @param overflow Previously calculated layout overflow

        @return Void. But the elements of the parent element will now have their `computedPos` ajusted as needed.
    */
    auto calculateAlignmentForElements(node::UIBase* parent, const glm::vec2 overflow) const -> void;


    /** @brief Calculates the layout overflow amount generated by the elements of the parent element.

        @note This shall be used only after the positioning and scaling pass have been done.
        @note If the layout contains any scrollbars, their size also needs to be taken into account (`shrinkScaleBy`).

        @param parent Element for which the overflow needs to be calculated
        @param shrinkScaleBy Optional parameter to shrink the parent computed scale area if needed
            (usually used to make room for scroll bars)

        @return Positive overflow value is there is any overflow. Negative value otherwise.
    */
    auto calculateElementOverflow(node::UIBase* parent, const glm::vec2 shrinkScaleBy = {}) const -> glm::vec2;


    // /** @brief Calculate the computed pos and scale of all the slider scrollbars of a UIPane/UIPane derivate.

    //     @note As some other functions need to know what impact the sliders have on the parent layout before hand,
    //         it is best for this function to be ran as earliest as possible.
    //     @note Sliders that do no have the scrollbar role are not affected by this.

    //     @param parent Element for which the slider child elements need to be positioned and scaled

    //     @return How much space on each axis the slider(s) occupy.
    // */
    // auto calculateSlidersScaleAndPos(node::UIPane* parent) const -> glm::vec2;


    // /** @brief Calculates the new computed pos for the parent's elements by adding in the scroll offset.

    //     @param parent Element for which the elements need to be offset
    //     @param offset The offset by which to move the child element's position
    // */
    // auto calculateElementsOffsetDueToScroll(node::UIPane* parent, const glm::ivec2 offset) const -> void;


    // /** @brief Calculates the `computedPos` and `computedScale` of a UISPlitPane element.

    //     @details Function calculates position and scale according to `setScale` relative values
    //         provided by the user and also take into account `minScale` and `maxScale` of each element.
    //         This will try to resize elements to fulfill min/max and scale as best as possible.
    //     @details Also if there's a handleIdx value set, relative scale values will be updated reflecting
    //         user's left/right/top/bottom dragging of the handle controlled panes.

    //     @param parent Element for which split pane elements need to be calculated
    //     @param handleIdx Currently active split pane handle
    //     @param mousePos Current mouse position
    // */
    // auto calculateSplitPaneElements(node::UISplitPane* parent, const uint32_t handleIdx,
    //     const glm::vec2 mousePos) const -> void;


    // /** @brief Calculates the `computedPos` of the child container of the dropdown.

    //     @param element Element for which to calculate child position
    // */
    // auto calculatePositionForDropdownElement(node::UIDropdown* element) const -> void;

private:
    struct SpacingDetails
    {
        glm::vec2 additionalStartPush{0, 0};
        glm::vec2 spaceBetween{0, 0};
    };

    /** @brief Calculates the spacing to be applied between parent elements in order to follow user set rule.

        @details This uses the `Spacing` option in oder to determine the additional start push the first element
            will experience as well as the gap that needs to be maintained between the elements.

        @param parent Parent for which the child elements spacing needs to be calculated
        @param shrinkScaleBy Optional parameter to shrink the parent computed scale area if needed
            (usually used to make room for scroll bars)

        @return Additional start element needed push offset and the space to keep between elements.
    */
    auto calculateSpacingOnAxis(node::UIBase* parent,
        const glm::vec2 shrinkScaleBy) const -> SpacingDetails;


    /** @brief Calculate the minimum computedScale needed for the parent element to perfectly fit around it's children.

        @note This function WILL NOT set any computedScale for any element, it just tries to compute the minimum
            gift-wrapped scale needed elsewhere.

        @details Function will try to compute the scale required for `parent` such that the child elements of it
            fit perfectly inside. Parent's padding/borders/margins are taken into consideration for this calculation.
            If a child element is itself of scale type FIT, then this function will recurse down on it until the end.
        @details Leaf child elements are REQUIRED to be of scale type PX, otherwise it is impossible to compute
            the FIT scale of the initial node.

        @param parent Parent element to wrap it's children around

        @return Minimum fit scale needed.
    */
    auto calculateFitScale(node::UIBase* parent) const -> glm::vec2;


    /** @brief Calculate the `computedScale` of parent child elements when the parent is of type GRID.

        @note This pass needs to be done before the positioning pass.
        @note Slider nodes with scrollbar role are ignored.

        @param parent Element for which the subelements need to be calculated
        @param shrinkScaleBy Optional parameter to shrink the parent computed scale area if needed
            (usually used to make room for scroll bars)
    */
    auto calculateScaleForGenericElementOfTypeGrid(node::UIBase* parent,
        const glm::vec2 shrinkScaleBy) const -> void;


    /** @brief Calculate the `computedPos` of parent child elements when the parent is of type GRID.

        @note This pass needs to be done after the scaling pass.
        @note Slider nodes with scrollbar role are ignored.

        @param parent Element for which the child elements need to be calculated
        @param shrinkScaleBy Optional parameter to shrink the parent computed scale area if needed
            (usually used to make room for scroll bars)
    */
    auto calculatePosForGenericElementOfTypeGrid(node::UIBase* parent,
        const glm::vec2 shrinkScaleBy) const -> void;


    /** @brief Calculates the start positions at which each row and column need to be placed at.

        @details Precomputed value will be used to know where to place each element in the grid
            and how much that element needs to be scaled by.
        @details Values are stored in a 1D array representing only the first column and row of the grid.
            Every other cell precomputed value can be extracted from that.

        @param parent Element for which the grid needs to be precomputed
        @param shrinkScaleBy Optional parameter to shrink the parent computed scale area if needed
            (usually used to make room for scroll bars)
    */
    auto calculatePrecomputedGridStartPos(node::UIBase* parent,
        const glm::vec2 shrinkScaleBy) const -> void;


    // /** @brief Simply calculate the scale of the handles of this split pane and return their accumulated size.

    //     @param parent SplitPane element for which to calculate handle sizes.

    //     @return Accumulated size of each handle.
    // */
    // auto calculateSplitPaneHandlesScale(node::UISplitPane* parent) const -> glm::vec2;


    // /** @brief Simply calculate the scale of the NON-handle elements of this split pane.

    //     @param parent SplitPane element for which to calculate NON-handle element sizes.
    // */
    // auto calculateSplitPaneNonHandleScale(node::UISplitPane* parent,
    //     const glm::vec2 handlesSize) const -> void;


    // /** @brief Try and apply scale corrections for the panes if needed.

    //     @details When resizing the split pane area we might end up with a pane going under or over
    //         the minimum, respectively maximum scale set. This will try to keep those constrains
    //         satisfied as best as possible.

    //     @param parent SplitPane element for which to apply corrections.
    //     @param handlesSize Previously calculated accumulated handles size.
    // */
    // auto applySplitPaneElementsScaleCorrection(node::UISplitPane* parent,
    //     const glm::vec2 handlesSize) const -> void;


    // /** @brief Simply calculate the position of all the elements of the split pane.

    //     @param parent SplitPane element for which to calculate positions.
    // */
    // auto calculateSplitPaneElementsPos(node::UISplitPane* parent) const -> void;


    // /** @brief Adjust the `handleIdx` controlled panes relative scale by the amount the user has dragged.

    //     @param parent SplitPane element for which to calculate positions.
    //     @param handleIdx Currently under drag pane handle
    //     @param handlesSize Total size of all the handles in the split pane
    //     @param mousePos Current mouse position
    // */
    // auto calculateSplitPaneRelativeValuesDueToDrag(node::UISplitPane* parent,
    //     const uint32_t handleIdx, const glm::vec2 handlesSize, const glm::vec2 mousePos) const -> void;


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
    utils::Logger log_{"BasicCalculator"};
};
} // namespace lav::core
