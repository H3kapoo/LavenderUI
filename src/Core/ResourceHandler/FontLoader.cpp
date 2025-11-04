#include "FontLoader.hpp"
#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Core/ResourceHandler/Font.hpp"

namespace lav::core
{
FontLoader& FontLoader::get()
{
    static FontLoader instance;
    return instance;
}

FontLoader::FontLoader()
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

FontPtr FontLoader::loadFont(const std::string& fontPath, const int32_t fontSize)
{
    std::string fontKey = fontPath + std::to_string(fontSize);
    if (fontPathToObject_.count(fontKey))
    {
        return fontPathToObject_.at(fontKey);
    }

    // std::packaged_task<FontPtr()> task([this, fontPath, fontSize]()
    // {
    //     return loadFontInternal(fontPath, fontSize);
    // });

    // auto futureTask = task.get_future();

    // if (BELoadingQueue::get().isThisMainThread()) { task(); }
    // /* This is not the main thread */
    // else { BELoadingQueue::get().pushTask(std::move(task)); }

    // fontPathToObject_[fontKey] = futureTask.get();
    fontPathToObject_[fontKey] = loadFontInternal(fontPath, fontSize);

    return fontPathToObject_.at(fontKey);
}

FontPtr FontLoader::loadFontInternal(const std::string& fontPath, const int32_t fontSize)
{
    FontPtr font = std::make_shared<Font>();
    font->fontSize = fontSize;
    font->fontPath = fontPath;

    if (fontSize < MIN_FONT_SIZE || fontSize > MAX_FONT_SIZE)
    {
        log_.error("Failed to load font: \"%s\". Size is out of bounds: %d. Will keep previous font size.",
            fontPath.c_str(), fontSize);
        return font;
    }

    FT_Face ftFace;
    if (FT_New_Face(ftLib_, fontPath.c_str(), 0, &ftFace))
    {
        log_.error("Failed to load font: \"%s\". Will keep previous font.", fontPath.c_str());
        return font;
    }

    FT_Set_Pixel_Sizes(ftFace, fontSize, fontSize);

    // /* Generate MAX_GLYPHS levels deep texture. */
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

    log_.debug("Loaded font texture {} with size {} from \"{}\"", font->textureId, fontSize, fontPath);

    /* Unbind texture and free FreeType resources */
    GPUBinder::get().bindIdToTextureType(GPUBinder::TextureType::Array2D, 0);
    FT_Done_Face(ftFace);

    return font;
}
} // namespace lav::core