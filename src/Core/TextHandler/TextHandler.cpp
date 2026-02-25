#include <LavenderUI/Core/TextHandler/TextHandler.hpp>

#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include "LavenderUI/Core/TextHandler/Common.hpp"
#include "LavenderUI/Core/TextHandler/TextBatchStore.hpp"
#include "LavenderUI/Utils/Logger.hpp"
#include <LavenderUI/Core/ResourceHandler/Font.hpp>
#include <LavenderUI/Core/ResourceHandler/Shader.hpp>
#include <LavenderUI/Core/ResourceHandler/ShaderLoader.hpp>
#include <LavenderUI/Core/ResourceHandler/FontLoader.hpp>
#include <LavenderUI/Core/ResourceHandler/MeshLoader.hpp>
#include <LavenderUI/Utils/Misc.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace lav::core
{
TextHandler::TextHandler(const fs::path& vertShaderPath, const fs::path& fragShaderPath)
    : options_()
    , storedText_()
    , textColor_(utils::hexToVec4("#141414ff"))
    , shader_(ShaderLoader::get().load(vertShaderPath, fragShaderPath))
    , font_(FontLoader::get().loadFont(core::DEFAULT_FONT_PATH))
    // , font_(FontLoader::get().loadFont(core::DEFAULT_FONT_PATH, 26))
    , mesh_(MeshLoader::get().loadQuad())
    , lastCharPos_(0, 0)
    , basePos_(0, 0)
    , maxTextBounds_(0, 0)
    , rollingPos_(0, 0)
{
    options_.lineHeight = core::DEFAULT_FONT_SIZE;
}

auto TextHandler::render(const glm::mat4& projection) -> void
{
    // TODO:
    // On Windows IGPU 620 at least, the max components we can have overall in a shader is 4096.
    // If we reserve buffers of 256, it will not work and that is why it was reduced to 128 comps.
    // It is suggested to move away from uploading huge chunks of matrix data to the GPU and use
    // UBOs or SSBO for large data chunks like these.
    // This may not happen on Linux Mesa or other newer intel igpus.
    if (storedText_.empty()) { return; }

    /* Setup base shader values. */
    mesh_.bind();
    shader_.bind();
    shader_.uploadVec4f("uColor", textColor_);
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadTexture2DArray("uTextureArray", 0, font_->textureId);

    renderBatches();
}

auto TextHandler::renderBatches() -> void
{
    utils::Logger log("aci");
    // log.warn("batch start");
    // log.warn("Batches rendered {}", getMaxTextBounds().x);

    basePos_ = layoutBounds_.pos;
    basePos_.y += font_->baseVerticalSep + font_->descender;
    basePos_ += (layoutBounds_.scale - maxTextBounds_) / 2;

    TextBatchStore::get().start();
    while (prepareNextBatch())
    {
        renderBatch();
    }
    TextBatchStore::get().end();

    /* Save where the last char was placed. */
    lastCharPos_ = basePos_;
}

auto TextHandler::renderBatch() -> void
{
    TextBatchStore& tbs = TextBatchStore::get();
    shader_.uploadIntv("uCharIndices", tbs.getGlyphs());
    shader_.uploadMat4v("uModelMatrices", tbs.getModels());
    core::GPUBinder::get().renderBoundQuadInstanced(tbs.getCurrentBatchSize());
}

auto TextHandler::prepareNextBatch() -> bool
{
    TextBatchStore& tbs = TextBatchStore::get();
    uint32_t globalCharIndex = tbs.getGlobalOffset();
    // if (globalCharIndex >= storedText_.size()) { return false; }
    if (globalCharIndex >= storedText_.size() || !isPosInsideBounds(basePos_)) { return false; }

    /* Clear previous batch data */
    tbs.clearBuffer();

    while (!tbs.isFull())
    {
        /* Handle last batch which might not be full.*/
        if (globalCharIndex >= storedText_.size()) { return true; }

        const auto& gd = font_->glyphData[storedText_[globalCharIndex]];

        tbs.setGlobalOffset(++globalCharIndex);

        /* Ajust char into position. */
        glm::ivec2 pos;
        pos.x = basePos_.x + gd.bearing.x;
        pos.y = basePos_.y - gd.bearing.y;

        if (options_.wrap && pos.x + gd.size.x > layoutBounds_.pos.x + layoutBounds_.scale.x)
        {
            basePos_.y += font_->baseVerticalSep;
            basePos_.x = layoutBounds_.pos.x;
            pos = basePos_;
            pos.y -= gd.bearing.y;
        }

        /* Advance is stored in 1/64ths of a pixel by FT lib for some reason. Need to bitshift right. */
        basePos_ += glm::ivec2{(gd.hAdvance >> 6), 0};

        /* Skip rendering any spaces, but take into account the advance. */
        if (gd.glyphCode == ' ') { continue; }

        glm::mat4 model{glm::mat4(1.0f)};
        model = glm::translate(model, glm::vec3(pos.x, pos.y, layoutBounds_.zIndex + 0.01f));
        model = glm::scale(model, glm::vec3(font_->fontSize, font_->fontSize, 1));

        tbs.push(gd.glyphCode, std::move(model));
    }

    return true;
}

auto TextHandler::getMaxTextBounds() -> glm::ivec2
{
    glm::ivec2 maxx{0, font_->baseVerticalSep};
    glm::ivec2 endPos{layoutBounds_.pos};
    for (const auto& chr : storedText_)
    {
        const auto& gd = font_->glyphData[chr];
        glm::ivec2 pos;
        pos.x = endPos.x + gd.bearing.x;
        pos.y = endPos.y - gd.bearing.y;

        if (options_.wrap && pos.x + gd.size.x > layoutBounds_.pos.x + layoutBounds_.scale.x)
        {
            endPos.y += font_->baseVerticalSep;
            endPos.x = layoutBounds_.pos.x;
            pos = endPos;
            pos.y -= gd.bearing.y;

            maxx.y += font_->baseVerticalSep;
        }

        /* Advance is stored in 1/64ths of a pixel by FT lib for some reason. Need to bitshift right. */
        endPos += glm::ivec2{(gd.hAdvance >> 6), 0};
        maxx.x = std::max(maxx.x, endPos.x - layoutBounds_.pos.x);
    }

    return maxx;
}

auto TextHandler::isPosInsideBounds(const glm::ivec2 pos) -> bool
{
    return (pos.y <= layoutBounds_.pos.y + layoutBounds_.scale.y)
        && (pos.x <= layoutBounds_.pos.x + layoutBounds_.scale.x);
}

auto TextHandler::alignText() -> void
{
    switch (options_.align)
    {
        case TextOptions::Align::LEFT:
            break;
        case TextOptions::Align::CENTER:
        {
            // for (auto& mat : renderBuffer_.glyphModel)
            // {
            //     mat[3][0] += static_cast<int32_t>(layoutBounds_.scale.x / 2 - lastCharPos_.x / 2);
            //     mat[3][1] += static_cast<int32_t>(layoutBounds_.scale.y / 2 - lastCharPos_.y / 2);
            // }
            break;
        }
        case TextOptions::Align::RIGHT:
            break;
    }
}

auto TextHandler::setDisplayBounds(const TextLayoutBounds& bounds) -> void
{
    if (layoutBounds_ != bounds)
    {
        maxTextBounds_ = getMaxTextBounds();
    }

    layoutBounds_ = bounds;
}

auto TextHandler::setTextColor(const glm::vec4& color) -> void
{
    textColor_ = color;
}

auto TextHandler::setText(const std::string& text) -> void
{
    storedText_ = text; // not efficient, use other means in the future
    maxTextBounds_ = getMaxTextBounds();
}

auto TextHandler::setFont(const fs::path& fontPath, const uint32_t size) -> void
{
    const FontPtr wantedFont = FontLoader::get().loadFont(fontPath, size);
    if (wantedFont->textureId) { font_ = wantedFont; }
}

auto TextHandler::setEllipsisEnabled(const uint32_t count) -> void
{
    options_.ellipsis = count;
}

auto TextHandler::setWrapEnabled(const bool value) -> void
{
    options_.wrap = value;
}

auto TextHandler::setTextAlign(const core::TextOptions::Align align) -> void
{
    options_.align = align;
}

auto TextHandler::getText() const -> std::string
{
    return storedText_;
}

auto TextHandler::getTextColor() const -> glm::vec4
{
    return textColor_;
}

auto TextHandler::getFont() const -> const Font&
{
    return *font_;
}
} // namespace lav::core
