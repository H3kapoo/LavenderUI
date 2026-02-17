#pragma once

#include <filesystem>

#include "LavenderUI/Core/TextHandler/TextOptions.hpp"
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
class BasicTextHandler
{
public:
    BasicTextHandler(const fs::path& vertShaderPath, const fs::path& fragShaderPath);

    auto render(const glm::mat4& projection) -> void;
    auto computeMaxSize() -> glm::vec2;

    auto setBounds(const glm::ivec2 pos, const glm::ivec2 scale, const int32_t zIndex) -> void;
    auto setTextColor(const glm::vec4& color) -> void;
    auto setText(const std::string& text) -> void;
    auto setFont(const fs::path& fontPath, const uint32_t size = 16) -> void;
    auto setTextAlign(const core::TextOptions::Align align) -> void;

    auto setBatchSize(const uint32_t size) -> void;
    auto setEllipsisEnabled(const uint32_t count) -> void;
    auto setWrapEnabled(const bool value) -> void;

    auto getText() const -> std::string;
    auto getTextColor() const -> glm::vec4;

private:
    auto updateZIndex() -> void;
    auto clearBufferAndReserve() -> void;
    auto alignText() -> void;

protected:
    auto layout() -> void;
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
    glm::ivec2 lastCharPos_;
    glm::ivec2 boundsStart_;
    glm::ivec2 boundsScale_;
    uint32_t batchSize_;
    int32_t zIndex_;
    TextOptions options_;
    uint32_t ellipsisCount_;
    bool isWrapEnabled_;
    bool dirty_{true};
};
} // namespace lav::core