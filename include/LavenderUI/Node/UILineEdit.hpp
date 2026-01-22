#pragma once

#include <optional>

#include "include/LavenderUI/Core/TextHandler/TextAttribs.hpp"
#include "include/LavenderUI/Node/UIBase.hpp"

namespace lav::node
{
/**
    @brief
    Class represeting UI element for displaying and editing a line of text.
*/
class UILineEdit : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UILineEdit, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UILineEdit);

    auto setText(const std::string& text) -> UILineEdit&;
    auto setFont(const std::filesystem::path& fontPath) -> void;

    auto getText() const -> std::string;

private:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(core::UIStatePtr& state) -> void override;

protected:
    core::TextAttribs textAttribs_;
    std::string placeholderText_;
    std::optional<glm::vec4> overrideColor_;
};
using UILineEditPtr = std::shared_ptr<UILineEdit>;
using UILineEditWPtr = std::weak_ptr<UILineEdit>;
using UILineEditPtrVec = std::vector<UILineEditPtr>;
} // namespace lav::node
