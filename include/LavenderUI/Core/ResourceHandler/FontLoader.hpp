#pragma once

#include <string>
#include <unordered_map>

#include <LavenderUI/Utils/Logger.hpp>
#include <LavenderUI/Core/ResourceHandler/Font.hpp>
#include <freetype/include/freetype/freetype.h>

namespace lav::core
{
class FontLoader
{
public:
    static FontLoader& get();

    FontPtr loadFont(const std::string& fontPath, const int32_t fontSize = 16);

private:
    FontLoader();
    ~FontLoader();

    FontPtr loadFontInternal(const std::string& fontPath, const int32_t fontSize);

    /* Cannot be copied or moved */
    FontLoader(const FontLoader&) = delete;
    FontLoader(FontLoader&&) = delete;
    FontLoader& operator=(const FontLoader&) = delete;
    FontLoader& operator=(FontLoader&&) = delete;

private:
    utils::Logger log_;
    FT_Library ftLib_;
    std::unordered_map<std::string, FontPtr> fontPathToObject_;
};
} // namespace lav::core