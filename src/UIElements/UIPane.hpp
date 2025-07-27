#pragma once

#include <memory>

#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UISlider.hpp"

namespace src::uielements
{
/**
    @brief:
        Pane GUI element that can be used as a container for other elements.
        Has the ability to scroll the elements if needed.

    @notes:
    (1) If a new element needs scrolling functionality, it's best to derive it from this.
    (2) If scroll is enabled on some axis, then there's a UISlider automatically added as the
        new child element of this.
*/
class UIPane : public UIBase
{
public:
    UIPane();
    UIPane(const std::type_index& type);
    virtual ~UIPane() = default;
    UIPane(const UIPane&) = delete;
    UIPane(UIBase&&) = delete;
    UIPane& operator=(const UIPane&) = delete;
    UIPane& operator=(UIPane&&) = delete;

    auto setScrollEnabled(const bool enableH, const bool enableV) -> UIPane&;
    auto setScrollSensitivityMultiplier(const float value) -> UIPane&;
    auto isVerticalOverflow() const -> bool;
    auto isHorizontalOverflow() const -> bool;
    auto getHorizontalSlider() const -> UISliderWPtr;
    auto getVerticalSlider() const -> UISliderWPtr;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UIPane);

protected:
    /**
        @brief:
            Update the current sliders with the new overflow value.

        @notes:
        (1) Sliders will dissappear on the appropriate axis if overflow is less or equal to zero.
            However they are still parented as long as scrolling is enabled.

        @params:
            overflow - the new overflow value

        @returns:
            True if by updating the overflow, the layout needs to be recalculated again.
            False otherwise.
    */
    auto updateSlidersWithOverflow(const glm::vec2& overflow) -> bool;

    /**
        @brief:
            Checks whether one of the active sliders of this object is the closest one to the
            mouse position and updates the state of the window if so.

        @notes:
        (1) Vertical slider is always prioritized in favor of the horizontal one.
        (2) If this function is not used while processing events, the sliders of this object will
            not be taken into consideration. Might be something you want.

        @params:
            state - window state that will be updated if needed
    */
    auto updateClosestSlider(state::UIWindowStatePtr& state) -> void;

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(state::UIWindowStatePtr& state) -> void override;

protected:
    UISliderPtr hSlider_;
    UISliderPtr vSlider_;
};
using UIPanePtr = std::shared_ptr<UIPane>;
using UIPaneWPtr = std::weak_ptr<UIPane>;
} // namespace src::uielements
