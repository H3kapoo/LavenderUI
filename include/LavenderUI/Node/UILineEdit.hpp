#pragma once

#include <optional>

#include <LavenderUI/Core/TextHandler/CaretTextHandler.hpp>
#include <LavenderUI/Node/UIBase.hpp>

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

    auto enableNumbericOnly(const bool enable = true) -> void;
    auto setText(const std::string& text) -> void;
    auto setFont(const std::filesystem::path& fontPath) -> void;
    auto setTextColor(const glm::vec4& color) -> void;

    auto getText() const -> std::string;

private:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(core::UIStatePtr& state) -> void override;

    auto performFiltering(const char codepoint) -> bool;

protected:
    core::CaretTextHandler textHandler_;
    std::string placeholderText_;
    std::optional<glm::vec4> overrideColor_;
    bool numericOnly_;
};
using UILineEditPtr = std::shared_ptr<UILineEdit>;
using UILineEditWPtr = std::weak_ptr<UILineEdit>;
using UILineEditPtrVec = std::vector<UILineEditPtr>;
} // namespace lav::node
