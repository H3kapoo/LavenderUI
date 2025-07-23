#include "PropsBase.hpp"

#include "vendor/glm/gtc/matrix_transform.hpp"
#include "vendor/glm/trigonometric.hpp"

namespace src::elementcomposable
{
auto PropsBase::setColor(const glm::vec4& value) -> void { color_ = value; }

auto PropsBase::getColor() const -> const glm::vec4& { return color_; }
} // namespace src::elementcomposable