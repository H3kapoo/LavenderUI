#pragma once

#include <memory>

#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UISlider.hpp"

namespace src::uielements
{
using namespace elementcomposable;

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

    auto enableScroll(const bool valueH, const bool valueV) -> UIPane&;
    auto setScrollSensitivity(const float value) -> UIPane&;
    auto isVerticalOverflow() const -> bool;
    auto isHorizontalOverflow() const -> bool;
    auto getHorizontalSlider() const -> UISliderWPtr;
    auto getVerticalSlider() const -> UISliderWPtr;

    INSERT_TYPEINFO(UIPane);

protected:
    auto updateSlidersWithOverflow(const glm::vec2& overflow) -> bool;
    auto updateClosestSlider(framestate::FrameStatePtr& state) -> void;

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(framestate::FrameStatePtr& state) -> void override;

protected:
    UISliderPtr hSlider_;
    UISliderPtr vSlider_;
};
using UIPanePtr = std::shared_ptr<UIPane>;
using UIPaneWPtr = std::weak_ptr<UIPane>;
} // namespace src::uielements
