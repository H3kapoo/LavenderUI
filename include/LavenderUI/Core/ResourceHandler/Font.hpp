#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include <glm/glm.hpp>

namespace lav::core
{
namespace fs = std::filesystem;

static constexpr int32_t MAX_GLYPHS        {256};
static constexpr int32_t DEFAULT_FONT_SIZE {16};
static constexpr int32_t MIN_FONT_SIZE     {10};
static constexpr int32_t MAX_FONT_SIZE     {88};
static const fs::path DEFAULT_FONT_PATH {"assets/fonts/Arial.ttf"};

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
    fs::path fontPath;
};
using FontPtr = std::shared_ptr<Font>;
} // namespace lav::core