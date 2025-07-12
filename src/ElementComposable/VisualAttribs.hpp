#pragma once

#include "vendor/glm/glm.hpp"

namespace src::elementcomposable
{
class VisualAttribs
{
public:
    VisualAttribs() = default;
    virtual ~VisualAttribs() = default;

    auto setColor(const glm::vec4& value) -> void;

    auto getColor() const -> const glm::vec4&;

protected:
    glm::vec4 color_{1.0f};
};
} // namespace src::elementcomposable