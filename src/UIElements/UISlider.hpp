#pragma once

#include "src/ElementComposable/LayoutAttribs.hpp"
#include "src/ElementComposable/VisualAttribs.hpp"
#include "src/UIElements/UIBase.hpp"

namespace src::uielements
{
using namespace elementcomposable;

class UISlider : public UIBaseCPRT<UISlider>
{
public:
    UISlider();
    ~UISlider() = default;
    UISlider(const UISlider&) = delete;
    UISlider(UIBase&&) = delete;
    UISlider& operator=(const UISlider&) = delete;
    UISlider& operator=(UISlider&&) = delete;

    auto getKnobLayout() -> LayoutAttribs&;
    auto getKnobVisual() -> VisualAttribs&;
    auto getScrollPercentage() -> float;
    auto getScrollValue() -> float;
    auto setScrollValue(const float value) -> void;
    auto setScrollFrom(const float value) -> void;
    auto setScrollTo(const float value) -> void;

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(framestate::FrameStatePtr& state, const elementevents::IEvent& e) -> void override;

    auto calculatePercentage(const glm::ivec2& mPos) -> float;
    auto calculateKnobPosition() -> void;

private:
    LayoutAttribs knobLayoutAttr_;
    VisualAttribs knobVisualAttr_;
    float percentage_{0.0f};
    float scrollFrom_{0.0f};
    float scrollTo_{100.0f};
    float scrollValue_{0.0f};
};
using UISliderPtr = std::shared_ptr<UISlider>;
using UISliderWPtr = std::weak_ptr<UISlider>;
} // namespace src::uielements
