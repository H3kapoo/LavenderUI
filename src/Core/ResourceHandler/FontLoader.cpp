#include <LavenderUI/Core/ResourceHandler/FontLoader.hpp>

#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/ResourceHandler/Font.hpp>

namespace lav::core
{
FontLoader& FontLoader::get()
{
    static FontLoader instance;
    return instance;
}

FontLoader::FontLoader()
    : log_(__func__)
{
    if (FT_Init_FreeType(&ftLib_))
    {
        log_.error("FreeType lib failed to load!");
    }
}

FontLoader::~FontLoader()
{
    FT_Done_FreeType(ftLib_);
    log_.debug("Deallocated.");
}

FontPtr FontLoader::loadFont(const fs::path& fontPath, const int32_t fontSize)
{
    // std::string fontKey = fontPath + std::to_string(fontSize);
    std::string fontKey = fontPath.string() + std::to_string(fontSize);
    if (fontPathToObject_.count(fontKey))
    {
        return fontPathToObject_.at(fontKey);
    }

    fontPathToObject_[fontKey] = loadFontInternal(fontPath, fontSize);

    return fontPathToObject_.at(fontKey);
}

FontPtr FontLoader::loadFontInternal(const fs::path& fontPath, const int32_t fontSize)
{
    FontPtr font = std::make_shared<Font>();
    font->fontSize = fontSize;
    font->fontPath = fontPath;

    if (fontSize < MIN_FONT_SIZE || fontSize > MAX_FONT_SIZE)
    {
        log_.error("Failed to load font: \"{}\". Size is out of bounds: {}. Will keep previous font size.",
            fontPath.string().c_str(), fontSize);
        return font;
    }

    FT_Face ftFace;
    if (FT_New_Face(ftLib_, fontPath.string().c_str(), 0, &ftFace))
    {
        log_.error("Failed to load font: \"{}\". Will keep previous font.", fontPath.string().c_str());
        return font;
    }

    FT_Set_Pixel_Sizes(ftFace, fontSize, fontSize);

    /* Generate MAX_GLYPHS levels deep texture. */
    GPUBinder::get().unpackAlignment();
    font->textureId = GPUBinder::get().createTexture(
        fontSize,
        fontSize,
        MAX_GLYPHS,
        GPUBinder::TextureType::Array2D,
        GPUBinder::ColorType::MONO,
        GPUBinder::TextureOptions{},
        nullptr);

    if (!font->textureId)
    {
        log_.error("Texture Id returned is zero!");
        return font;
    }

    GPUBinder::get().bindIdToTextureType(GPUBinder::TextureType::Array2D, font->textureId);

    FT_Int32 load_flags = FT_LOAD_RENDER;
    for (int32_t i = 32; i < MAX_GLYPHS; i++)
    {
        /* Loads i'th char in font atlas */
        if (FT_Load_Char(ftFace, i, load_flags))
        {
            log_.error("Error loading char code: %d", i);
            continue;
        }

        GPUBinder::get().bufferTextureData(
            ftFace->glyph->bitmap.width,
            ftFace->glyph->bitmap.rows,
            i,
            GPUBinder::TextureType::Array2D,
            GPUBinder::ColorType::MONO,
            ftFace->glyph->bitmap.buffer);

        Font::GlyphData ch =
        {
            .glyphCode = uint32_t(i),
            .hAdvance = ftFace->glyph->advance.x,
            .size = glm::ivec2(ftFace->glyph->bitmap_left + ftFace->glyph->bitmap.width,
                ftFace->glyph->bitmap_top + ftFace->glyph->bitmap.rows),
            .bearing = glm::ivec2(ftFace->glyph->bitmap_left, ftFace->glyph->bitmap_top)
        };

        font->glyphData[i] = ch;
    }

    log_.debug("Loaded font texture {} with size {} from \"{}\"", font->textureId, fontSize,
        fontPath.string().c_str());

    /* Unbind texture and free FreeType resources */
    GPUBinder::get().bindIdToTextureType(GPUBinder::TextureType::Array2D, 0);
    FT_Done_Face(ftFace);

    return font;
}
} // namespace lav::core