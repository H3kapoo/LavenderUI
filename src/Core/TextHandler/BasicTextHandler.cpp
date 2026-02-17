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
    : shader_(ShaderLoader::get().load(vertShaderPath, fragShaderPath))
    // , font_(FontLoader::get().loadFont(core::DEFAULT_FONT_PATH))
    , font_(FontLoader::get().loadFont(core::DEFAULT_FONT_PATH, 26))
    , mesh_(MeshLoader::get().loadQuad())
    , soaBuffer_()
    , storedText_()
    , textColor_(utils::hexToVec4("#141414ff"))
    , batchSize_(200)
{}

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
    shader_.uploadMat4v("uModelMatrices", soaBuffer_.glyphModel);
    shader_.uploadIntv("uCharIndices", soaBuffer_.glyphCode);
    shader_.uploadTexture2DArray("uTextureArray", 0, font_->textureId);
    core::GPUBinder::get().renderBoundQuadInstanced(storedText_.size());
}

auto BasicTextHandler::computeMaxSize() -> glm::vec2
{
    glm::vec2 size{0, 0};
    for (const uint8_t ch : storedText_)
    {
        const auto& cp = font_->glyphData[ch];
        size.x += cp.hAdvance >> 6;
        size.y = std::max(size.y, (float)cp.bearing.y);
    }
    return size;
}

auto BasicTextHandler::layout() -> void
{
    if (!dirty_) { return; }

    // glm::ivec2 start{startPos_.x, startPos_.y};
    // const glm::vec2 p = boundsStart_ + boundsScale_ / 2 - ms / 2;
    // glm::ivec2 nextStartPos{p.x + layout_.padding.left, p.y + layout_.padding.right};
    // glm::ivec2 start{boundsStart_.x + layout_.padding.left, boundsStart_.y + layout_.padding.right};
    // glm::ivec2 nextStartPos{p.x, p.y};
    soaBuffer_.glyphModel.clear();
    soaBuffer_.glyphModel.reserve(storedText_.size());

    glm::ivec2 nextStartPos{boundsStart_.x, boundsStart_.y};
    glm::ivec2 boundsEnd{boundsStart_ + boundsScale_};

    float inc = 0.1;
    glm::ivec2 pos{0, 0};
    glm::ivec2 textBegin{0, 999999};
    glm::ivec2 textEnd{0, 0};

    int32_t lowestPoint{0};
    int32_t highestPoint{9999};
    int32_t maxBearingY{0};
    int32_t maxUnderlineY{0};
    bool wrapped{false};

    utils::Logger log("aici");
    for (const uint8_t chr : storedText_)
    {
        const auto& gd = font_->glyphData[chr];
        // if (chr == ' ')
        // {
        //     nextStartPos.x += (gd.hAdvance >> 6);
        //     continue;
        // }

        pos.x = nextStartPos.x + gd.bearing.x;
        if (isWrapEnabled_ && pos.x + gd.size.x > boundsEnd.x)
        {
            nextStartPos.x = boundsStart_.x;
            nextStartPos.y += font_->fontSize;

            // log.warn("lh {}", lineHeight);
        }

        pos.x = nextStartPos.x + gd.bearing.x;
        pos.y = nextStartPos.y - gd.bearing.y;// + ( wrapped ? font_->fontSize : 0);

        highestPoint = std::min(highestPoint, pos.y);
        lowestPoint = std::max(lowestPoint, pos.y + gd.size.y);
        maxBearingY = std::max(maxBearingY, gd.bearing.y);
        maxUnderlineY = std::max(maxUnderlineY, gd.size.y - gd.bearing.y);

        glm::mat4 model{glm::mat4(1.0f)};
        model = glm::translate(model, glm::vec3(pos.x, pos.y, zIndex_ + inc));
        model = glm::scale(model, glm::vec3(font_->fontSize, font_->fontSize, 1));

        if (textBegin.x == 0)
        {
            textBegin.x = pos.x;
            // textBegin.y = pos.y;
        }

        // textEnd.x = std::max(textEnd.x, pos.x + gd.size.x);
        // textEnd.y = std::max(textEnd.y, pos.y + gd.size.y);

        /* Advance is stored in 1/64ths of a pixel by FT lib for some reason. Need to bitshift right. */
        nextStartPos.x += (gd.hAdvance >> 6);

        soaBuffer_.glyphModel.emplace_back(std::move(model));
        inc += 0.1f;
    }

    /* Push them such that they are top aligned with start bound Y. */
    // linesHeight = std::max(linesHeight, maxBearingY);
    // for (int32_t lineChr = lineStart; lineChr < soaBuffer_.glyphModel.size(); lineChr++)
    // {
    //     auto& mat = soaBuffer_.glyphModel[lineChr];
    //     // mat[3][1] += maxBearingY;
    //     mat[3][1] += font_->fontSize;
    // }
    // lastCharPos_ = textEnd - textBegin;
    // lastCharPos_.x = textEnd.x - textBegin.x;
    // lastCharPos_.y = 0;
    // lastCharPos_.y = maxBearingY + maxUnderlineY;

    for (auto& mat : soaBuffer_.glyphModel)
    {
        // mat[3][1] += font_->fontSize;
        // mat[3][1] += maxBearingY;
        // mat[3][1] += (highestPoint - boundsStart_.y);
        mat[3][1] += (boundsStart_.y - highestPoint);
    }

    // log.warn("highest {} lowest {}", highestPoint, lowestPoint);
    // log.warn("aa {}", maxUnderlineY + maxBearingY);
    lastCharPos_.y = lowestPoint - highestPoint;
    // lastCharPos_.y = maxUnderlineY + maxBearingY;

    alignText();

    // utils::Logger log("aici");
    // log.warn("size is {} {}", storedText_, lastCharPos_.x);
    dirty_ = false;
}

auto BasicTextHandler::alignText() -> void
{
    switch (options_.align)
    {
        case TextOptions::Align::LEFT:
            break;
        case TextOptions::Align::CENTER:
        {
            for (auto& mat : soaBuffer_.glyphModel)
            {
                // mat[3][0] += static_cast<int32_t>(boundsScale_.x / 2 - lastCharPos_.x / 2);
                // mat[3][1] += static_cast<int32_t>(boundsScale_.y / 2 - lastCharPos_.y);
                mat[3][1] += static_cast<int32_t>(boundsScale_.y / 2 - lastCharPos_.y / 2);
            }
            break;
        }
        case TextOptions::Align::RIGHT:
            break;
    }
}

auto BasicTextHandler::fillRenderBatch() -> void
{
    dirty_ = true;

    soaBuffer_.glyphCode.clear();
    soaBuffer_.glyphCode.reserve(storedText_.size());

    for (const uint8_t chr : storedText_)
    {
        const auto& gd = font_->glyphData[chr];
        soaBuffer_.glyphCode.emplace_back(gd.glyphCode);
    }
}

auto BasicTextHandler::updateZIndex() -> void
{
    /*
        In a 4X4 matrix in openGL, scale lives along the matrix diagonal and position lives
        down the last column:

        S_X NOP NOP P_X
        NOP S_Y NOP P_Y
        NOP NOP S_Z P_Z
        NOP NOP NOP NOP

        However when reading this as opengl wants it, we have to transpose the matrix
        aka rows become columns and vice versa:

        mat[row][col] -> mat[col][row]
    */
    // float inc_ = 0.1;
    // for (auto& mat : soaBuffer_.glyphModel)
    // {
    //     mat[3][2] = startPos_.z + 1 + inc_;
    //     inc_ += 0.1f;
    // }
}

auto BasicTextHandler::clearBufferAndReserve() -> void
{
}

auto BasicTextHandler::setBounds(const glm::ivec2 pos, const glm::ivec2 scale,
    const int32_t zIndex) -> void
{
    if (boundsStart_.x != pos.x || boundsStart_.y != pos.y
        || boundsScale_.x != scale.x || boundsScale_.y != scale.y
        || zIndex_ != zIndex)
    {
            dirty_ = true;
    }

    boundsStart_ = pos;
    boundsScale_ = scale;
    zIndex_ = zIndex;

    layout();
}

auto BasicTextHandler::setTextColor(const glm::vec4& color) -> void
{
    textColor_ = color;
}

auto BasicTextHandler::setText(const std::string& text) -> void
{
    storedText_ = text; // not efficient, use other means in the future
    fillRenderBatch();
}

auto BasicTextHandler::setFont(const fs::path& fontPath, const uint32_t size) -> void
{
    FontPtr wantedFont = FontLoader::get().loadFont(fontPath, size);
    if (wantedFont->textureId) { font_ = wantedFont; }
}

auto BasicTextHandler::setBatchSize(const uint32_t size) -> void
{
    batchSize_ = size;
}

auto BasicTextHandler::setEllipsisEnabled(const uint32_t count) -> void
{
    ellipsisCount_ = count;
}

auto BasicTextHandler::setWrapEnabled(const bool value) -> void
{
    isWrapEnabled_ = value;
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