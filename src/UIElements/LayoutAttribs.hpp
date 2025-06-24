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

    auto computeViewBox(const LayoutAttribs& parentAttribs) -> void;
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
    glm::vec2 pos{0}, scale{100.0f, 100.0f};
    glm::vec2 viewPos{0}, viewScale{0};
    uint32_t index{1};
    float angle{30.0f};
    bool isCustomLevel{false};

private:
    glm::mat4 transform_{glm::mat4{1}};
};
} // namespace src::uielements