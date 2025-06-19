#pragma once

#include "src/UIElements/UIBase.hpp"

namespace src::uielements
{
class UIButton : public UIBase
{
public:
    UIButton(const std::string& name);
    ~UIButton() = default;
    UIButton(const UIButton&) = delete;
    UIButton(UIBase&&) = delete;
    UIButton& operator=(const UIButton&) = delete;
    UIButton& operator=(UIButton&&) = delete;

    // friend auto operator<<(std::ostream& out, const UIBasePtr&) -> std::ostream&;

private:
    auto render(const glm::mat4& projection) -> void override;

protected:
};
using UIButtonPtr = std::shared_ptr<UIButton>;
using UIButtonWPtr = std::weak_ptr<UIButton>;
} // namespace src::uielements


// template<>
// struct std::formatter<src::uielements::UIBasePtr> : std::formatter<std::string_view>
// {
//     auto format(const src::uielements::UIBasePtr& obj, format_context& ctx) const
//     {
//         std::ostringstream os;
//         os << obj;
//         return std::formatter<std::string_view>::format(std::move(os).str(), ctx);
//     }
// };