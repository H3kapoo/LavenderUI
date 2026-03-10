#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace lav::core
{
/** @brief Common place for defining text oriented utilitary structures.*/

struct TextLayoutBounds
{
    glm::ivec2 pos{0, 0};
    glm::ivec2 scale{0, 0};
    uint32_t zIndex{0};

    auto operator==(const TextLayoutBounds& other) -> bool
    {
        return pos == other.pos && scale == other.scale && zIndex == other.zIndex;
    }
};

class TextOptions
{
public:
    enum class Align
    {
        LEFT, CENTER, RIGHT
    };

    Align align{Align::LEFT};
    uint32_t ellipsis{0};
    uint32_t lineHeight{0};
    bool wrap{false};
};
} // namespace lav::core