#pragma once

#include <chrono>
#include <condition_variable>
#include <thread>

#include "include/LavenderUI/Core/ResourceHandler/Shader.hpp"
#include "thirdparty/glm/glm.hpp"

namespace lav::core
{
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
