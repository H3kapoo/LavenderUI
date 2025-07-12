#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "src/ResourceLoaders/Font.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "vendor/glm/glm.hpp"

namespace src::elementcomposable
{
class TextAttribs
{
struct TextSoA;

public:
    TextAttribs();
    auto computeMaxSize() const -> glm::vec2;

    auto setFont(const std::filesystem::path& fontPath) -> void;
    auto setText(std::string text) -> void;
    auto setPosition(const glm::ivec2& pos) -> void;
    auto setValidBounds(const glm::vec2& start, const glm::vec2& scale) -> void;

    auto getShader() -> resourceloaders::Shader&;
    auto getText() const -> std::string;
    auto getBuffer() const -> const TextSoA&;
    auto getFont() const -> const resourceloaders::FontPtr&;

private:
    struct TextSoA
    {
        TextSoA() {}
        TextSoA(const uint32_t size)
        {
            glyphCode.reserve(size);
            model.reserve(size);
        }

        std::vector<int32_t> glyphCode;
        std::vector<glm::mat4> model;
    };

private:
    resourceloaders::Shader shader_;
    TextSoA buffer_;
    glm::vec2 pos_{0.0f};
    std::string text_;
    resourceloaders::FontPtr font_;
};
} // namespace src::elementcomposable
