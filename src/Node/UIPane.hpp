#pragma once

#include "src/Node/InternalUse/UIScroll.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::node
{
/**
    @brief Pane GUI element that can be used as a container for other elements.
        Has the ability to scroll the elements if needed.

    @note If a new element needs scrolling functionality, it's best to derive it from this.
    @note If scroll is enabled on some axis, then there's a UISlider automatically added as the
            new child element of this.
*/
class UIPane : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIPane, "elemVert.glsl", "elemFrag.glsl");

    auto setScrollEnabled(const bool enableH, const bool enableV) -> UIPane&;
    auto setScrollSensitivityMultiplier(const float value) -> UIPane&;
    auto isVerticalOverflow() const -> bool;
    auto isHorizontalOverflow() const -> bool;
    auto getHorizontalSlider() const -> UISliderWPtr;
    auto getVerticalSlider() const -> UISliderWPtr;

protected:
    /**
        @brief Update the current sliders with the new overflow value.

        @note Sliders will dissappear on the appropriate axis if overflow is less or equal to zero.
            However they are still parented as long as scrolling is enabled.

        @param overflow The new overflow value

        @return True if by updating the overflow, the layout needs to be recalculated again.
            False otherwise.
    */
    auto updateSlidersWithOverflow(const glm::vec2& overflow) -> bool;

    /**
        @brief Checks whether one of the active sliders of this object is the closest one to the
            mouse position and updates the state of the window if so.

        @note Vertical slider is always prioritized in favor of the horizontal one.
        @note If this function is not used while processing events, the sliders of this object will
            not be taken into consideration. Might be something you want.

        @param state - Window state that will be updated if needed
    */
    auto updateClosestSlider(node::UIStatePtr& state) -> void;

private:
    virtual auto render(const glm::mat4& projection) -> void override;
    virtual auto layout() -> void override;
    virtual auto event(node::UIStatePtr& state) -> void override;

protected:
    UIScrollPtr hScroll_;
    UIScrollPtr vScroll_;

private:
    /** Note: Ideally layout will not be recalculated due to slider addition/removal more than once,
        but sometimes, due to rounding errors or pure entropy, it will get stuck. Cap max retries. */
    int32_t updateTriesCount_{0};
    int32_t maxUpdateTries_{2};
};
using UIPanePtr = std::shared_ptr<UIPane>;
using UIPaneWPtr = std::weak_ptr<UIPane>;
using UIPanePtrVec = std::vector<UIPanePtr>;
} // namespace lav::node
