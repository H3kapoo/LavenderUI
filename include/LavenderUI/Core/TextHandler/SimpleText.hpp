#pragma once

#include <filesystem>

#include <LavenderUI/Core/ResourceHandler/Font.hpp>
#include <LavenderUI/Core/ResourceHandler/Mesh.hpp>
#include <LavenderUI/Core/ResourceHandler/Shader.hpp>
#include <LavenderUI/Utils/Logger.hpp>
#include <glm/glm.hpp>

namespace lav::core
{
namespace fs = std::filesystem;

/**
    @brief
    Class that deals with managing a simple line of text with no caret or editing.
*/
class SimpleText
{
public:
    SimpleText(const fs::path& vertShaderPath, const fs::path& fragShaderPath);

    auto render(const glm::mat4& projection) -> void;
    auto computeMaxSize() -> glm::vec2;

    auto setTextColor(const glm::vec4& color) -> void;
    auto setText(const std::string& text) -> void;
    auto setFont(const fs::path& fontPath, const uint32_t size = 16) -> void;
    auto setBatchSize(const uint32_t size) -> void;
    auto setAnchorPos(const glm::ivec2 pos) -> void;
    auto setStartZIndex(const uint32_t index) -> void;

    auto getText() const -> std::string;

private:
    auto fillRenderBatch() -> void;

private:
    struct StructOfArraysBuffer
    {
        std::vector<int32_t> glyphCode;
        std::vector<glm::mat4> glyphModel;
    };

private:
    Shader shader_;
    FontPtr font_; // why ptr?
    Mesh mesh_;

protected:
    StructOfArraysBuffer soaBuffer_;
    std::string storedText_;
    glm::vec4 textColor_;
    glm::ivec3 startPos_;
    uint32_t batchSize_;
};
} // namespace lav::core