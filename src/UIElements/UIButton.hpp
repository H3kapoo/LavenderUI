#pragma once

#include "src/ElementComposable/TextAttribs.hpp"
#include "src/UIElements/UIBase.hpp"

namespace src::uielements
{
using namespace elementcomposable;

/**
    @brief Simple generic button which generic click functionality.

    @note Can be inherited to create custom button types.
*/
class UIButton : public UIBase
{
public:
    UIButton();
    UIButton(const std::type_index& type);
    ~UIButton() = default;
    UIButton(const UIButton&) = delete;
    UIButton(UIBase&&) = delete;
    auto operator=(const UIButton&) -> UIButton& = delete;
    auto operator=(UIButton&&) -> UIButton& = delete;

    auto setFont(const std::filesystem::path& fontPath) -> void;
    auto setText(const std::string& text) -> void;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UIButton);

private:
    virtual auto render(const glm::mat4& projection) -> void override;
    virtual auto layout() -> void override;
    virtual auto event(state::UIWindowStatePtr& state) -> void override;

protected:
    TextAttribs textAttribs_;
    glm::vec4 originalColor_{utils::hexToVec4("#c73e3eff")};
    glm::vec4 onEnterColor_{utils::hexToVec4("#c95959ff")};
    glm::vec4 onClickColor_{utils::hexToVec4("#c41c1cff")};
};
using UIButtonPtr = std::shared_ptr<UIButton>;
using UIButtonWPtr = std::weak_ptr<UIButton>;
using UIButtonPtrVec = std::vector<UIButtonPtr>;
} // namespace src::uielements
