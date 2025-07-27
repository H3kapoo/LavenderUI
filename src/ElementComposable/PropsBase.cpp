#include "PropsBase.hpp"

namespace src::elementcomposable
{
auto PropsBase::setColor(const glm::vec4& value) -> PropsBase& { color_ = value; return *this; }

auto PropsBase::getColor() const -> const glm::vec4& { return color_; }
} // namespace src::elementcomposable