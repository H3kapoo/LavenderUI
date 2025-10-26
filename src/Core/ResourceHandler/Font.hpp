#pragma once

#include <string>
#include <memory>
#include <vector>

#include "vendor/glm/glm.hpp"

namespace lav::core
{
static constexpr int32_t MAX_GLYPHS        {256};
static constexpr int32_t DEFAULT_FONT_SIZE {16};
static constexpr int32_t MIN_FONT_SIZE     {10};
static constexpr int32_t MAX_FONT_SIZE     {88};
static const std::string DEFAULT_FONT_PATH {"/home/hekapoo/Documents/probe/move_stuff/assets/fonts/Arial.ttf"};

struct Font
{
    struct GlyphData
    {
        uint32_t glyphCode;
        int64_t hAdvance;
        glm::ivec2 size;
        glm::ivec2 bearing;
    };

    GlyphData glyphData[MAX_GLYPHS];
    uint32_t textureId{0};
    int32_t fontSize{DEFAULT_FONT_SIZE};
    std::string fontPath;
};
using FontPtr = std::shared_ptr<Font>;
} // namespace lav::core