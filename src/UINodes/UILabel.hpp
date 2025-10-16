#pragma once

#include <optional>

#include "src/ElementComposable/TextAttribs.hpp"
#include "src/UINodes/UIBase.hpp"

namespace src::uinodes
{
using namespace elementcomposable;

/**
    @brief Label element for displaying text.
*/
class UILabel : public UIBase
{
public:
    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UILabel);

    UILabel();
    UILabel(UIBaseInitData&& data);
    ~UILabel() = default;
    UILabel(const UILabel&) = delete;
    UILabel(UIBase&&) = delete;
    auto operator=(const UILabel&) -> UILabel& = delete;
    auto operator=(UILabel&&) -> UILabel& = delete;

    auto setColor(const glm::vec4& value) -> UILabel&;
    auto setBorderColor(const glm::vec4& value) -> UILabel&;
    auto setText(const std::string& text) -> UILabel&;
    auto setFont(const std::filesystem::path& fontPath) -> void;

    auto getColor() const -> const glm::vec4&;
    auto getBorderColor() const -> const glm::vec4&;

private:
    virtual auto render(const glm::mat4& projection) -> void override;
    virtual auto layout() -> void override;
    virtual auto event(state::UIStatePtr& state) -> void override;

protected:
    TextAttribs textAttribs_;
    std::optional<glm::vec4> overrideColor_{std::nullopt};
    glm::vec4 baseColor_{utils::hexToVec4("#979797ff")};
    glm::vec4 borderColor_{utils::hexToVec4("#979797ff")};
};
using UILabelPtr = std::shared_ptr<UILabel>;
using UILabelWPtr = std::weak_ptr<UILabel>;
using UILabelPtrVec = std::vector<UILabelPtr>;
} // namespace src::uinodes
