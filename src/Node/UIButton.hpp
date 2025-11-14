#pragma once

#include <optional>

#include "src/Core/TextHandler/TextAttribs.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UILabel.hpp"

namespace lav::node
{
/**
    @brief Simple generic button which generic click functionality.

    @note Can be inherited to create custom button types.
*/
class UIButton : public UIBase
{
public:
    /* Mandatory typeinfo */
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIButton, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UIScroll);

    auto setClickedColor(const glm::vec4& color) -> UIButton&;
    auto setHoveredColor(const glm::vec4& color) -> UIButton&;
    auto setEnabled() -> UIButton&;
    auto setDisabled() -> UIButton&;
    auto setText(const std::string& text) -> UIButton&;

    auto isEnabled() -> bool;
    auto getColor() const -> const glm::vec4&;
    auto getBorderColor() const -> const glm::vec4&;

private:
    virtual auto render(const glm::mat4& projection) -> void override;
    virtual auto layout() -> void override;
    virtual auto event(UIStatePtr& state) -> void override;

protected:
    UILabelPtr label_{utils::make<UILabel>()};
    std::optional<glm::vec4> overrideColor_{std::nullopt};
    glm::vec4 clickedColor_{utils::hexToVec4("#c95959ff")};
    glm::vec4 hoveredColor_{utils::hexToVec4("#c41c1cff")};
    bool isBtnEnabled_{true};
};
using UIButtonPtr = std::shared_ptr<UIButton>;
using UIButtonWPtr = std::weak_ptr<UIButton>;
using UIButtonPtrVec = std::vector<UIButtonPtr>;
} // namespace lav::node
