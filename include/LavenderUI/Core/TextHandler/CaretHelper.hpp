#pragma once

#include <chrono>
#include <condition_variable>
#include <thread>

#include <glm/glm.hpp>
#include <LavenderUI/Core/ResourceHandler/Shader.hpp>

namespace lav::core
{
/**
    @brief Responsible for managing the lifetime and attributes of the caret blinking symbol.

    @note Singleton class as there can only be one caret at the time for now.
    @note Multiline caret selection will break with current implementation but it's not needed right now.
*/
class CaretHelper
{
public:
    static auto get() -> CaretHelper&;

    auto render(const glm::mat4& projection) -> void;
    auto start() -> void;
    auto stop() -> void;
    auto requestKeepAlive() -> void;
    auto setBlinkTime(const std::chrono::milliseconds& ms) -> void;
    auto setCaretColor(const glm::vec4 color) -> void;
    auto setCaretScale(const glm::ivec2 scale) -> void;
    auto setCaretPos(const glm::ivec2 pos) -> void;

private:
    CaretHelper();
    ~CaretHelper();
    CaretHelper(const CaretHelper&) = delete;
    CaretHelper(CaretHelper&&) = delete;
    auto operator =(const CaretHelper&) -> CaretHelper& = delete;
    auto operator =(CaretHelper&&) -> CaretHelper& = delete;

    auto startBlinkLogic() -> void;
    auto getCaretTransform() -> const glm::mat4;

private:
    Shader caretShader_;
    glm::vec4 caretColor_;
    glm::ivec2 caretScale_;
    glm::ivec2 caretPos_;
    double lastBlinkTime_;
    double lastKeepAliveTimeSec_;
    std::chrono::milliseconds blinkIntervalMs_;
    bool renderCaret_;
    bool started_;
    std::unique_ptr<std::jthread> blinkThread_;
    std::condition_variable cv_;
    std::mutex mtx_;
};
} // namespace lav::core
