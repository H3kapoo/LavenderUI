#pragma once

namespace lav::core
{
class TextOptions
{
public:
    enum class Align
    {
        LEFT, CENTER, RIGHT
    };

    Align align{Align::LEFT};
};
} // namespace lav::core