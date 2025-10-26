#pragma once

#include "src/ElementComposable/TextAttribs.hpp"
#include "src/UIElements/UIBase.hpp"

namespace src::uielements
{
using namespace elementcomposable;

/**
    @brief Slider GUI element that can act as basic a basic slider and the scroll bar of a UIPane.
*/
class UISlider : public UIBase
{
public:
    UISlider();
    ~UISlider() = default;
    UISlider(const UISlider&) = delete;
    UISlider(UIBase&&) = delete;
    auto operator=(const UISlider&) -> UISlider& = delete;
    auto operator=(UISlider&&) -> UISlider& = delete;

    auto getScrollPercentage() -> float;
    auto getScrollValue() -> float;
    auto getKnobLayout() -> LayoutBase&;
    auto getKnobProps() -> PropsBase&;
    auto getTextAttribs() -> TextAttribs&;

    auto setScrollValue(const float value) -> void;
    auto setScrollFrom(const float value) -> void;
    auto setScrollTo(const float value) -> void;
    auto setScrollSensitivity(const float value) -> void;
    auto setFont(const std::filesystem::path& fontPath) -> void;
    auto setText(const std::string& text) -> void;
    auto setInvertAxis(const bool value) -> void;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UISlider);

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(state::UIStatePtr&state) -> void override;

    auto calculatePercentage(const glm::ivec2& mPos) -> float;
    auto calculateKnobPosition() -> void;

public:
    static uint32_t scrollTagId; /** @brief Tag used to check if this Slider is actually a scroll bar. */
    static uint32_t scrollIndexOffset /** @brief Scroll bars need to start at a higher z index, */;

protected:
    LayoutBase knobLayout_;
    PropsBase knobProps_;
    TextAttribs textAttribs_;
    float percentage_{0.0f};
    float scrollFrom_{0.0f};
    float scrollTo_{100.0f};
    float scrollValue_{0.0f};

private:
    glm::ivec2 offsetToKnobCenter_{0.0f, 0.0f};
    bool invertVertical_{false}; /* false - starts from bottom; true - starts from top */
    float sensitivity_{2.0f};
};
using UISliderPtr = std::shared_ptr<UISlider>;
using UISliderWPtr = std::weak_ptr<UISlider>;
} // namespace src::uielements
