#include "TextAttribs.hpp"
#include "src/ResourceLoaders/Font.hpp"
#include "src/ResourceLoaders/FontLoader.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

namespace src::elementcomposable
{
TextAttribs::TextAttribs()
    : shader_(resourceloaders::ShaderLoader::get().load(
        "assets/shaders/basicTextVert.glsl", "assets/shaders/basicTextFrag.glsl"))
    , font_(resourceloaders::FontLoader::get().loadFont(resourceloaders::DEFAULT_FONT_PATH))
    
{}

auto TextAttribs::setFont(const std::filesystem::path& fontPath) -> void
{
    // fontPath_ = fontPath;
}

auto TextAttribs::setText(std::string text) -> void
{
    text_ = std::move(text);

    buffer_ = std::move(TextSoA(text.size()));
    glm::vec2 startPos{pos_};
    float z{0};
    float mockIndex = 4;
    for (const uint8_t c : text_)
    {
        const auto& glyphData = font_->glyphData[c];
        const float x = startPos.x + glyphData.bearing.x;
        const float y = startPos.y - glyphData.bearing.y;// + glyphData.textBounds.y;// + fontData->fontSize * lineNo;

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3{x, y, mockIndex + z});
        modelMatrix = glm::scale(modelMatrix, glm::vec3{font_->fontSize, font_->fontSize, 1});

        startPos.x += (glyphData.hAdvance >> 6);
        z += 0.01f;

        buffer_.glyphCode.emplace_back(glyphData.glyphCode);
        buffer_.model.emplace_back(std::move(modelMatrix));
    }
}

auto TextAttribs::setPosition(const glm::vec2& pos) -> void
{
    pos_ = pos;
    setText(text_);
}

auto TextAttribs::getText() const -> std::string
{
    return text_;
}

auto TextAttribs::getBuffer() const -> const TextSoA&
{
    return buffer_;
}

auto TextAttribs::getFont() const -> const resourceloaders::FontPtr&
{
    return font_;
}

auto TextAttribs::getShader() -> resourceloaders::Shader&
{
    return shader_;
}
} // namespace src::elementcomposable