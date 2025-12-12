#pragma once

#include "src/Node/UIBase.hpp"
#include "src/Node/UILabel.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::node
{
/**
    @brief
    Class representing an UI element that can act as slider for a single value.
*/
class UISlider : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UISlider, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UISlider);

    auto getScrollPercentage() -> float;
    auto getScrollValue() -> float;
    auto getKnobBaseLayoutData() -> core::LayoutBase&;
    auto getLabel() -> UILabelWPtr;

    auto setScrollValue(const float value) -> void;
    auto setScrollFrom(const float value) -> void;
    auto setScrollTo(const float value) -> void;
    auto setScrollSensitivity(const float value) -> void;
    auto setKnobColor(const glm::vec4& value) -> void;
    auto setFont(const std::filesystem::path& fontPath) -> void;
    auto setText(const std::string& text) -> void;
    auto setInvertAxis(const bool value) -> void;

protected:
    auto onRender(const glm::mat4& projection) -> void override;
    auto onLayout() -> void override;
    auto onEvent(node::UIStatePtr& state) -> void override;

private:
    auto calculatePercentage(const glm::ivec2& mPos) -> float;

protected:
    core::LayoutBase knobLayout_;
    glm::vec4 knobColor_;
    UILabelPtr label_;
    float percentage_;
    float scrollFrom_;
    float scrollTo_;
    float scrollValue_;

private:
    glm::ivec2 distToKnobCenter_;
    float sensitivity_;
    bool invertVertical_; /* false - starts from bottom; true - starts from top */
};
using UISliderPtr = std::shared_ptr<UISlider>;
using UISliderWPtr = std::weak_ptr<UISlider>;
} // namespace lav::node
