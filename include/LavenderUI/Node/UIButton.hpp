#pragma once

#include <optional>

#include "include/LavenderUI/Node/UIBase.hpp"
#include "include/LavenderUI/Node/UILabel.hpp"

namespace lav::node
{
/**
    @brief
    Class representing generic button UI Element.
*/
class UIButton : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIButton, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UIScroll);

    auto setClickedColor(const glm::vec4& color) -> UIButton&;
    auto setHoveredColor(const glm::vec4& color) -> UIButton&;
    auto setEnabled() -> UIButton&;
    auto setDisabled() -> UIButton&;
    auto setText(const std::string& text) -> UIButton&;

    auto isEnabled() -> bool;
    auto getText() const -> std::string;
    auto getColor() const -> const glm::vec4&;
    auto getBorderColor() const -> const glm::vec4&;

protected:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(core::UIStatePtr& state) -> void override;
    virtual auto onResetToDefault() -> void override;

protected:
    UILabelPtr label_;
    std::optional<glm::vec4> overrideColor_;
    glm::vec4 clickedColor_;
    glm::vec4 hoveredColor_;
    bool isBtnEnabled_;
};
using UIButtonPtr = std::shared_ptr<UIButton>;
using UIButtonWPtr = std::weak_ptr<UIButton>;
using UIButtonPtrVec = std::vector<UIButtonPtr>;
} // namespace lav::node
