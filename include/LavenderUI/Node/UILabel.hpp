#pragma once

#include <optional>

#include <LavenderUI/Core/TextHandler/TextHandler.hpp>
#include <LavenderUI/Node/UIBase.hpp>

namespace lav::node
{
/**
    @brief
    Class represeting UI element for displaying one line of text.
*/
class UILabel : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UILabel, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UILabel);

    auto setText(const std::string& text) -> void;
    auto setFont(const std::filesystem::path& fontPath, const uint32_t size = 16) -> void;
    auto setFontSize(const uint32_t size) -> void;
    auto setTextColor(const glm::vec4& color) -> void;
    auto setTextAlign(const core::TextOptions::Align align) -> void;
    auto setTextEllipsis(const uint32_t count = 3) -> void;
    auto setScaleToTextSize() -> void;

    auto getText() const -> std::string;
    auto getTextColor() const -> glm::vec4;

private:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(core::UIStatePtr& state) -> void override;

protected:
    core::TextHandler textHandler_;
    std::optional<glm::vec4> overrideColor_;
};
using UILabelPtr = std::shared_ptr<UILabel>;
using UILabelWPtr = std::weak_ptr<UILabel>;
using UILabelPtrVec = std::vector<UILabelPtr>;
} // namespace lav::node
