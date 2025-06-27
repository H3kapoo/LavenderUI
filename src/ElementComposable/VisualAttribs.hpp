#pragma once

#include "vendor/glm/glm.hpp"

namespace src::elementcomposable
{
class VisualAttribs
{
public:
    VisualAttribs() = default;
    ~VisualAttribs() = default;
    VisualAttribs(const VisualAttribs&) = delete;
    VisualAttribs(VisualAttribs&&) = delete;
    VisualAttribs& operator=(const VisualAttribs&) = delete;
    VisualAttribs& operator=(VisualAttribs&&) = delete;

public:
    glm::vec4 color{1.0f};
};
} // namespace src::elementcomposable