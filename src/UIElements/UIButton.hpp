#pragma once

#include "src/UIElements/UIBase.hpp"

namespace src::uielements
{
class UIButton : public UIBase
{
public:
    UIButton();
    ~UIButton() = default;
    UIButton(const UIButton&) = delete;
    UIButton(UIBase&&) = delete;
    UIButton& operator=(const UIButton&) = delete;
    UIButton& operator=(UIButton&&) = delete;

    INSERT_TYPEINFO(UIButton);

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(framestate::FrameStatePtr& state) -> void override;
};
using UIButtonPtr = std::shared_ptr<UIButton>;
using UIButtonWPtr = std::weak_ptr<UIButton>;
} // namespace src::uielements
