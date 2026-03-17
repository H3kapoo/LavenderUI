#pragma once

#include <LavenderUI/Node/UILabel.hpp>
#include <LavenderUI/Node/UIBase.hpp>

namespace lav::node
{
/**
    @brief
    Class represeting UI element for displaying and editing a line of text.
*/
class UILineEdit : public UILabel
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UILineEdit, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UILineEdit);

    auto setNumbericOnly(const bool enable) -> void;
    auto setCaretColor(const glm::vec4& color) -> void;
    auto setBlinkTime(const std::chrono::milliseconds& ms) -> void;
    auto setEditable(const bool editable) -> void;

private:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(core::UIStatePtr& state) -> void override;

    auto notifyTyping() -> void;
    auto appendAtCaretPos(const char chr) -> void;
    auto removeAtCaretPos() -> void;
    auto setFocused(const bool focused) -> void;
    auto performFiltering(const char codepoint) -> bool;
    auto handleCaretRendering(const glm::mat4& projection) -> void;

protected:
    std::string placeholderText_;
    glm::vec4 caretColor_;
    bool numericOnly_;
    bool isEditable_;
    bool focused_;
    std::chrono::milliseconds blinkIntervalMs_;
};
using UILineEditPtr = std::shared_ptr<UILineEdit>;
using UILineEditWPtr = std::weak_ptr<UILineEdit>;
using UILineEditPtrVec = std::vector<UILineEditPtr>;
} // namespace lav::node
