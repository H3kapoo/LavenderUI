#pragma once

#include "src/UIElements/UIBase.hpp"

namespace src::uielements
{
/**
    @brief:
        Simple generic button which generic click functionality.
    
    @notes:
    (1) Can be inherited to create custom button types.
*/
class UIButton : public UIBase
{
public:
    UIButton();
    ~UIButton() = default;
    UIButton(const UIButton&) = delete;
    UIButton(UIBase&&) = delete;
    UIButton& operator=(const UIButton&) = delete;
    UIButton& operator=(UIButton&&) = delete;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UIButton);

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(state::UIWindowStatePtr& state) -> void override;
};
using UIButtonPtr = std::shared_ptr<UIButton>;
using UIButtonWPtr = std::weak_ptr<UIButton>;
} // namespace src::uielements
