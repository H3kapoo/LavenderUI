#pragma once

#include "vendor/glm/glm.hpp"

namespace src::uielements
{
class LayoutAttribs
{
public:
    struct BoundingBox
    {
        glm::vec2 start;
        glm::vec2 end;
    };

public:
    LayoutAttribs() = default;
    ~LayoutAttribs() = default;
    LayoutAttribs(const LayoutAttribs&) = delete;
    LayoutAttribs(LayoutAttribs&&) = delete;
    LayoutAttribs& operator=(const LayoutAttribs&) = delete;
    LayoutAttribs& operator=(LayoutAttribs&&) = delete;

    // contains size of the drawable as sent to the shader: includes border, padding, content are
    auto getDrawableBox() -> BoundingBox;
    // auto getFullBox() -> BoundingBox;
    auto getTransform() -> const glm::mat4&;

public:
    glm::ivec4 margin{0};
    glm::ivec4 padding{0};
    glm::ivec4 border{0};
    glm::ivec4 borderRadius{0};
    glm::ivec4 shadow{0}; // no ideea of it's use so far
    glm::vec3 pos{0}, scale{100.0f, 100.0f, 1.0f};
    glm::vec3 viewPos{0}, viewScale{0};
    float angle{30.0f};

private:
    glm::mat4 transform_{glm::mat4{1}};
};
} // namespace src::uielements