#pragma once

#include <filesystem>

#include "LavenderUI/Core/TextHandler/Common.hpp"
#include <LavenderUI/Core/ResourceHandler/Font.hpp>
#include <LavenderUI/Core/ResourceHandler/Mesh.hpp>
#include <LavenderUI/Core/ResourceHandler/Shader.hpp>
#include <LavenderUI/Utils/Logger.hpp>

namespace lav::core
{
namespace fs = std::filesystem;

/**
    @brief
    Class that deals with managing a simple line of text with no caret or editing.
*/
class TextHandler
{
public:
    TextHandler(const fs::path& vertShaderPath, const fs::path& fragShaderPath);

    auto render(const glm::mat4& projection) -> void;

    auto setDisplayBounds(const TextLayoutBounds& bounds) -> void;
    auto setTextColor(const glm::vec4& color) -> void;
    auto setText(const std::string& text) -> void;
    auto setFont(const fs::path& fontPath, const uint32_t size = 16) -> void;
    auto setFontSize(const uint32_t size) -> void;
    auto setTextAlign(const core::TextOptions::Align align) -> void;

    auto setEllipsisEnabled(const uint32_t count) -> void;
    auto setWrapEnabled(const bool value) -> void;

    auto getText() const -> std::string;
    auto getTextColor() const -> glm::vec4;
    auto getFont() const -> const Font&;
    auto getMaxTextBounds() const -> glm::ivec2;

private:
    TextHandler(const TextHandler&) = delete;
    TextHandler(TextHandler&&) = delete;
    auto operator=(const TextHandler&) -> TextHandler& = delete;
    auto operator=(TextHandler&&) -> TextHandler& = delete;

    auto renderBatches() -> void;
    auto renderBatch() -> void;
    auto prepareNextBatch() -> bool;
    auto alignText() -> void;
    auto computeMaxTextBounds() -> void;
    auto isPosInsideBounds(const glm::ivec2 pos) -> bool;

protected:
    TextOptions options_;
    TextLayoutBounds layoutBounds_;
    std::string storedText_;
    glm::vec4 textColor_;
    glm::ivec2 lastCharPos_;
    glm::ivec2 basePos_;
    glm::ivec2 maxTextBounds_;

private:
    Shader shader_;
    FontPtr font_; // why ptr?
    Mesh mesh_;
};
} // namespace lav::core