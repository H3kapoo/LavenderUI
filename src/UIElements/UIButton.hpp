#pragma once

#include "src/UIElements/UIBase.hpp"
#include "src/ElementComposable/TextAttribs.hpp"

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
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(state::UIWindowStatePtr& state) -> void override;

protected:
    TextAttribs textAttribs_;
};
using UIButtonPtr = std::shared_ptr<UIButton>;
using UIButtonWPtr = std::weak_ptr<UIButton>;
} // namespace src::uielements
