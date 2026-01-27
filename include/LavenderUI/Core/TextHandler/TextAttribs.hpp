#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "include/LavenderUI/Core/ResourceHandler/Font.hpp"
#include "include/LavenderUI/Core/ResourceHandler/Shader.hpp"
#include "thirdparty/glm/glm.hpp"

namespace lav::core
{
class TextAttribs
{
struct TextSoA;

public:
    TextAttribs();
    auto computeMaxSize() const -> glm::vec2;

    auto setFont(const std::filesystem::path& fontPath) -> void;
    auto setText(std::string text) -> void;
    auto appendChar(const char chr, const int32_t pos = -1) -> void;
    auto eraseChar(const int32_t pos = -1) -> void;
    auto getPosAtCursor(const glm::ivec2 mousePos) -> uint32_t;

    auto setPosition(const glm::ivec3& pos) -> void;
    auto setValidBounds(const glm::vec2& start, const glm::vec2& scale) -> void;

    auto getShader() -> Shader&;
    auto getText() const -> std::string;
    auto getBuffer() const -> const TextSoA&;
    auto getFont() const -> const FontPtr&;

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
    Shader shader_;
    TextSoA buffer_;
    std::string text_;
    glm::vec3 pos_;
    FontPtr font_;
};
} // namespace lav::core
