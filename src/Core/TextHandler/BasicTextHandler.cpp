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
    , font_(FontLoader::get().loadFont(core::DEFAULT_FONT_PATH))
    , mesh_(MeshLoader::get().loadQuad())
    , soaBuffer_()
    , storedText_()
    , textColor_(utils::hexToVec4("#141414ff"))
    , startPos_(0)
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

auto BasicTextHandler::fillRenderBatch() -> void
{
    soaBuffer_.glyphCode.clear();
    soaBuffer_.glyphModel.clear();
    soaBuffer_.glyphCode.reserve(storedText_.size());
    soaBuffer_.glyphModel.reserve(storedText_.size());

    glm::ivec2 start{startPos_.x, startPos_.y};

    auto ms = computeMaxSize();
    for (const uint8_t chr : storedText_)
    {
        const auto& gd = font_->glyphData[chr];
        const float cx = start.x + gd.bearing.x;
        const float cy = start.y - gd.bearing.y + ms.y;

        glm::mat4 model{glm::mat4(1.0f)};
        model = glm::translate(model, glm::vec3(cx, cy, startPos_.z));
        model = glm::scale(model, glm::vec3(font_->fontSize, font_->fontSize, 1));

        /* Advance is stored in 1/64ths of a pixel by FT lib by some reason. Need to bitshift right. */
        start.x += (gd.hAdvance >> 6);

        soaBuffer_.glyphCode.emplace_back(gd.glyphCode);
        soaBuffer_.glyphModel.emplace_back(std::move(model));
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
    float inc_ = 0.1;
    for (auto& mat : soaBuffer_.glyphModel)
    {
        mat[3][2] = startPos_.z + inc_;
        inc_ += 0.1f;
    }
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

auto BasicTextHandler::setAnchorPos(const glm::ivec2 pos) -> void
{
    /* Z is reserved as render index. */
    startPos_.x = pos.x;
    startPos_.y = pos.y;
    fillRenderBatch();
}

auto BasicTextHandler::setStartZIndex(const uint32_t index) -> void
{
    startPos_.z = index;
    updateZIndex();
}

auto BasicTextHandler::getText() const -> std::string
{
    return storedText_;
}
} // namespace lav::core