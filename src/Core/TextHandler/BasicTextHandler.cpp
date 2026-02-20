#include "LavenderUI/Core/TextHandler/Common.hpp"
#include "LavenderUI/Core/TextHandler/TextBatcher.hpp"
#include "LavenderUI/Utils/Logger.hpp"
#include <LavenderUI/Core/TextHandler/BasicTextHandler.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/ResourceHandler/Font.hpp>
#include <LavenderUI/Core/ResourceHandler/Shader.hpp>
#include <LavenderUI/Core/ResourceHandler/ShaderLoader.hpp>
#include <LavenderUI/Core/ResourceHandler/FontLoader.hpp>
#include <LavenderUI/Core/ResourceHandler/MeshLoader.hpp>
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::core
{
BasicTextHandler::BasicTextHandler(const fs::path& vertShaderPath, const fs::path& fragShaderPath)
    : options_()
    , storedText_()
    , textColor_(utils::hexToVec4("#141414ff"))
    , shader_(ShaderLoader::get().load(vertShaderPath, fragShaderPath))
    , font_(FontLoader::get().loadFont(core::DEFAULT_FONT_PATH))
    // , font_(FontLoader::get().loadFont(core::DEFAULT_FONT_PATH, 26))
    , mesh_(MeshLoader::get().loadQuad())
    // , batchSize_(128)
{
    options_.lineHeight = core::DEFAULT_FONT_SIZE;
}

auto BasicTextHandler::render(const glm::mat4& projection) -> void
{
    // TODO:
    // On Windows IGPU 620 at least, the max components we can have overall in a shader is 4096.
    // If we reserve buffers of 256, it will not work and that is why it was reduced to 128 comps.
    // It is suggested to move away from uploading huge chunks of matrix data to the GPU and use
    // UBOs or SSBO for large data chunks like these.
    // This may not happen on Linux Mesa or other newer intel igpus.
    if (storedText_.empty()) { return; }

    mesh_.bind();
    shader_.bind();
    shader_.uploadVec4f("uColor", textColor_);
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadTexture2DArray("uTextureArray", 0, font_->textureId);

    utils::Logger log("aci");

    lastCharPos_ = layoutBounds_.pos;
    TextBatcher::get().reset();
    while (prepareNextBatch())
    {
        renderBatch(projection);
    }
}

auto BasicTextHandler::prepareNextBatch() -> bool
{
    // utils::Logger log("aci");
    const auto currCount = TextBatcher::get().getTotalCount();
    if (currCount >= storedText_.size()) { return false; }

    /* Clear previous batch data */
    TextBatcher::get().clearBuffer();

    const auto batchSize = TextBatcher::get().getLimit();
    glm::ivec2 pos{0, 0};
    for (uint32_t batchCharIdx = 0; batchCharIdx < batchSize; ++batchCharIdx)
    {
        uint32_t globalCharIndex = currCount + batchCharIdx;
        /* Handle last batch which might not be full.*/
        if (globalCharIndex >= storedText_.size()) { return true; }

        const auto& gd = font_->glyphData[storedText_[globalCharIndex]];
        const auto nextPos = getNextPosition(gd, lastCharPos_);

        pos.x = nextPos.x + gd.bearing.x;
        pos.y = nextPos.y - gd.bearing.y + font_->fontSize;

        glm::mat4 model{glm::mat4(1.0f)};
        model = glm::translate(model, glm::vec3(pos.x, pos.y, layoutBounds_.zIndex + 0.01f));
        model = glm::scale(model, glm::vec3(font_->fontSize, font_->fontSize, 1));

        TextBatcher::get().push(gd.glyphCode, std::move(model));

        /* Advance is stored in 1/64ths of a pixel by FT lib for some reason. Need to bitshift right. */
        lastCharPos_ = nextPos + glm::ivec2{(gd.hAdvance >> 6) + gd.bearing.x, 0};
    }

    return true;
}

auto BasicTextHandler::getNextPosition(const Font::GlyphData& data, glm::ivec2 lcp) -> glm::ivec2
{
    glm::ivec2 pos{lcp};
    pos.x = lcp.x + data.bearing.x;
    if (options_.wrap && pos.x + data.size.x >= layoutBounds_.pos.x + layoutBounds_.scale.x)
    {
        pos.x = layoutBounds_.pos.x;
        pos.y += options_.lineHeight;
    }

    return pos;
}

auto BasicTextHandler::renderBatch(const glm::mat4& projection) -> void
{
    shader_.uploadIntv("uCharIndices", TextBatcher::get().getGlyphs());
    shader_.uploadMat4v("uModelMatrices", TextBatcher::get().getModels());
    core::GPUBinder::get().renderBoundQuadInstanced(TextBatcher::get().getCurrentBatchSize());
}

auto BasicTextHandler::layout() -> void
{
    // renderBuffer_.glyphModel.clear();
    // renderBuffer_.glyphModel.reserve(storedText_.size());

    // glm::ivec2 nextStartPos{layoutBounds_.pos};
    // glm::ivec2 boundsPosEnd{layoutBounds_.pos + layoutBounds_.scale};

    // glm::ivec2 pos{0, 0};

    // renderedTextMinMax_.reset();

    // // utils::Logger log("aici");
    // for (const uint8_t chr : storedText_)
    // {
    //     const auto& gd = font_->glyphData[chr];

    //     pos.x = nextStartPos.x + gd.bearing.x;
    //     if (options_.wrap && pos.x + gd.size.x > boundsPosEnd.x)
    //     {
    //         nextStartPos.x = layoutBounds_.pos.x;
    //         nextStartPos.y += options_.lineHeight;
    //     }

    //     pos.x = nextStartPos.x + gd.bearing.x;
    //     pos.y = nextStartPos.y - gd.bearing.y;

    //     renderedTextMinMax_.min.x = std::min(renderedTextMinMax_.min.x, pos.x);
    //     renderedTextMinMax_.max.x = std::max(renderedTextMinMax_.max.x, pos.x + gd.size.x);

    //     renderedTextMinMax_.min.y = std::min(renderedTextMinMax_.min.y, pos.y);
    //     renderedTextMinMax_.max.y = std::max(renderedTextMinMax_.max.y, pos.y + gd.size.y);

    //     glm::mat4 model{glm::mat4(1.0f)};
    //     model = glm::translate(model, glm::vec3(pos.x, pos.y, layoutBounds_.zIndex));
    //     model = glm::scale(model, glm::vec3(font_->fontSize, font_->fontSize, 1));

    //     /* Advance is stored in 1/64ths of a pixel by FT lib for some reason. Need to bitshift right. */
    //     nextStartPos.x += (gd.hAdvance >> 6);

    //     renderBuffer_.glyphModel.emplace_back(std::move(model));
    // }

    // for (auto& mat : renderBuffer_.glyphModel)
    // {
    //     mat[3][1] += (layoutBounds_.pos.y - renderedTextMinMax_.min.y);
    // }

    // lastCharPos_.x = renderedTextMinMax_.max.x - renderedTextMinMax_.min.x;
    // lastCharPos_.y = renderedTextMinMax_.max.y - renderedTextMinMax_.min.y;

    // log.warn("size {} {}", renderBounds_.min.x, renderBounds_.max.x);
    // alignText();
}

auto BasicTextHandler::alignText() -> void
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

auto BasicTextHandler::setDisplayBounds(const TextLayoutBounds& bounds) -> void
{
    layoutBounds_ = bounds;
}

auto BasicTextHandler::setTextColor(const glm::vec4& color) -> void
{
    textColor_ = color;
}

auto BasicTextHandler::setText(const std::string& text) -> void
{
    storedText_ = text; // not efficient, use other means in the future
}

auto BasicTextHandler::setFont(const fs::path& fontPath, const uint32_t size) -> void
{
    FontPtr wantedFont = FontLoader::get().loadFont(fontPath, size);
    if (wantedFont->textureId) { font_ = wantedFont; }
}

auto BasicTextHandler::setEllipsisEnabled(const uint32_t count) -> void
{
    options_.ellipsis = count;
}

auto BasicTextHandler::setWrapEnabled(const bool value) -> void
{
    options_.wrap = value;
}

auto BasicTextHandler::setTextAlign(const core::TextOptions::Align align) -> void
{
    options_.align = align;
}

auto BasicTextHandler::getText() const -> std::string
{
    return storedText_;
}

auto BasicTextHandler::getTextColor() const -> glm::vec4
{
    return textColor_;
}
} // namespace lav::core
