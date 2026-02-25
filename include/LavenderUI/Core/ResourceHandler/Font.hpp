#pragma once

#include <memory>

#include <glm/glm.hpp>

#include <LavenderUI/Core/Config.hpp>

namespace lav::core
{
static constexpr int32_t MAX_GLYPHS        {256};
static constexpr int32_t DEFAULT_FONT_SIZE {16};
static constexpr int32_t MIN_FONT_SIZE     {10};
static constexpr int32_t MAX_FONT_SIZE     {88};
// static const fs::path DEFAULT_FONT_PATH {Config::fontsPath / "Arial.ttf"};
// static const fs::path DEFAULT_FONT_PATH {Config::fontsPath / "cmr10.ttf"};
// static const fs::path DEFAULT_FONT_PATH {Config::fontsPath / "UbuntuNerdFont-Regular.ttf"};
static const fs::path DEFAULT_FONT_PATH {Config::fontsPath / "LiberationSerif-Regular.ttf"};

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
    int32_t ascender{0};
    int32_t descender{0};
    int32_t baseVerticalSep{0};
    fs::path fontPath;
};
using FontPtr = std::shared_ptr<Font>;
} // namespace lav::core