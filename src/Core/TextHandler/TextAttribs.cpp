#include "include/LavenderUI/Core/TextHandler/TextAttribs.hpp"

#include "include/LavenderUI/Core/ResourceHandler/Font.hpp"
#include "include/LavenderUI/Core/ResourceHandler/FontLoader.hpp"
#include "include/LavenderUI/Core/ResourceHandler/ShaderLoader.hpp"
#include "thirdparty/glm/gtc/matrix_transform.hpp"

namespace lav::core
{
TextAttribs::TextAttribs()
    : shader_(ShaderLoader::get().load(
        "assets/shaders/basicTextVert.glsl", "assets/shaders/basicTextFrag.glsl"))
    , buffer_()
    , text_()
    , pos_(0.0f)
    , font_(FontLoader::get().loadFont(core::DEFAULT_FONT_PATH))

{}

auto TextAttribs::computeMaxSize() const -> glm::vec2
{
    glm::vec2 size{0, 0};
    for (const uint8_t ch : text_)
    {
        const auto& cp = font_->glyphData[ch];
        size.x += cp.hAdvance >> 6;
        size.y = std::max(size.y, (float)cp.bearing.y);
    }
    return size;
}

auto TextAttribs::setFont(const std::filesystem::path& fontPath) -> void
{
    (void)fontPath;
    // fontPath_ = fontPath;
}

auto TextAttribs::setText(std::string text) -> void
{
    text_ = std::move(text);

    buffer_ = std::move(TextSoA(text.size()));
    glm::ivec2 startPos{pos_};
    float z{0.1f};
    // float mockIndex = 10;
    // this supports only one line for now
    // obviously this needs to be done only if the text changes
    const glm::ivec2 textBounds = computeMaxSize();
    for (const uint8_t c : text_)
    {
        const auto& glyphData = font_->glyphData[c];
        const float x = startPos.x + glyphData.bearing.x;
        const float y = startPos.y - glyphData.bearing.y + textBounds.y;

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3{x, y, pos_.z + z});
        modelMatrix = glm::scale(modelMatrix, glm::vec3{font_->fontSize, font_->fontSize, 1});

        startPos.x += (glyphData.hAdvance >> 6);
        z += 0.01f;

        buffer_.glyphCode.emplace_back(glyphData.glyphCode);
        buffer_.model.emplace_back(std::move(modelMatrix));
    }
}

auto TextAttribs::getPosAtCursor(const glm::ivec2 mousePos) -> uint32_t
{
    return 0;
}

auto TextAttribs::appendChar(const char chr, const int32_t pos) -> void
{
    (void)pos;
    setText(getText() + chr);
}

auto TextAttribs::eraseChar(const int32_t) -> void
{
    text_.pop_back();
}

auto TextAttribs::setPosition(const glm::ivec3& pos) -> void
{
    pos_ = pos;
    // only if the position really changes..
    setText(text_);
}

auto TextAttribs::setValidBounds(const glm::vec2& start, const glm::vec2& scale) -> void
{
    (void)scale;
    (void)start;
}

auto TextAttribs::getText() const -> std::string { return text_; }
auto TextAttribs::getBuffer() const -> const TextSoA& { return buffer_; }
auto TextAttribs::getFont() const -> const FontPtr& { return font_; }
auto TextAttribs::getShader() -> Shader& { return shader_; }
} // namespace lav::core