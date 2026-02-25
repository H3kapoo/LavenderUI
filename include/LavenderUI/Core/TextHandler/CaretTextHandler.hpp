#pragma once

#include <filesystem>

#include <LavenderUI/Core/TextHandler/TextHandler.hpp>
#include <LavenderUI/Utils/Logger.hpp>

namespace lav::core
{
/**
    @brief
    Class that deals with rendering of a piece of text and also of the caret that comes along with
    and editable text zone.
*/
class CaretTextHandler : public TextHandler
{
public:
    CaretTextHandler(const fs::path& vertShaderPath, const fs::path& fragShaderPath);

    auto render(const glm::mat4& projection) -> void;
    auto simulateMouseAt(const glm::ivec2 pos) -> void;
    auto notifyTyping() -> void;
    auto appendAtCaretPos(const char chr) -> void;
    auto removeAtCaretPos() -> void;
    auto moveCaretLeft(const uint32_t step = 1) -> void;
    auto moveCaretRight(const uint32_t step = 1) -> void;

    auto setText(const std::string& text) -> void;
    auto setFocused(const bool focused) -> void;
    auto setCaretColor(const glm::vec4& color) -> void;
    auto setBatchSize(const uint32_t size) -> void;
    auto setBlinkTime(const std::chrono::milliseconds& ms) -> void;
    auto setEditable(const bool editable) -> void;

    auto getText() const -> std::string;

private:
    auto handleCaretRendering(const glm::mat4& projection) -> void;
    auto getCaretTransform() -> const glm::mat4;

private:
    utils::Logger log_;
    glm::vec4 caretColor_;
    std::chrono::milliseconds blinkIntervalMs_;
    bool focused_;
    bool isEditable_;
};
} // namespace lav::core