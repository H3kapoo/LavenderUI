#pragma once

#include "vendor/glm/glm.hpp"

namespace src::elementcomposable
{
class PropsBase
{
/**
    @brief Base class for any generic element property.
*/
public:
    PropsBase() = default;
    virtual ~PropsBase() = default;

    auto setColor(const glm::vec4& value) -> PropsBase&;
    auto setBorderColor(const glm::vec4& value) -> PropsBase&;

    auto getColor() const -> const glm::vec4&;
    auto getBorderColor() const -> const glm::vec4&;

protected:
    glm::vec4 color_{1.0f};
    glm::vec4 borderColor_{1.0f};
};
} // namespace src::elementcomposable