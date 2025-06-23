#include "UIButton.hpp"
#include "src/UIElements/UIBase.hpp"

namespace src::uielements
{
UIButton::UIButton(const std::string& name)
    : UIBase(name, typeid(UIButton))
{}

auto UIButton::render(const glm::mat4& projection) -> void
{

}

auto UIButton::layout() -> void
{

}
} // namespace src::uielements
