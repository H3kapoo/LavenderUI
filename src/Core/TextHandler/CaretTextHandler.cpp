#include <LavenderUI/Core/TextHandler/CaretTextHandler.hpp>

#include <chrono>

#include <LavenderUI/Core/TextHandler/CaretHelper.hpp>

namespace lav::core
{
CaretTextHandler::CaretTextHandler(const fs::path& vertShaderPath, const fs::path& fragShaderPath)
    : BasicTextHandler(vertShaderPath, fragShaderPath)
    , log_("TextHandler")
    , blinkIntervalMs_(std::chrono::milliseconds(500))
    , focused_(false)
    , isEditable_(false)
{}

auto CaretTextHandler::render(const glm::mat4& projection) -> void
{
    BasicTextHandler::render(projection);

    /* Check if caret needs to be displayed. */
    if (!isEditable_ || !focused_) { return; }

    glm::ivec2 caretPos_;
    caretPos_.x = startPos_.x + computeMaxSize().x;
    caretPos_.y = startPos_.y + computeMaxSize().y / 2.0f - 20 / 2.0f;
    CaretHelper::get().setCaretPos(caretPos_);
    CaretHelper::get().render(projection);
}

auto CaretTextHandler::simulateMouseAt(const glm::ivec2 pos) -> void
{
    // const glm::ivec2 scale = computeMaxSize();
    // const bool isMouseInside = (pos.x >= startPos_.x && pos.x <= startPos_.x + scale.x)
    //     && (pos.y >= startPos_.y && pos.y <= startPos_.y + scale.y);

    // isMouseInside ? focused_ = true : focused_ = false;

    // if (isMouseInside && !blinkThread_)
    // {
    //     blinkThread_ = std::make_unique<std::jthread>(
    //         [this]()
    //         {
    //             while (focused_)
    //             {
    //                 core::WindowBinder::get().requestEmptyEvent();
    //                 std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //             }
    //         });
    // }
}

auto CaretTextHandler::notifyTyping() -> void
{
    CaretHelper::get().requestKeepAlive();
}

auto CaretTextHandler::appendAtCaretPos(const char chr) -> void
{
    if (!isEditable_) { return; }
    storedText_ += chr;
    notifyTyping();
}

auto CaretTextHandler::removeAtCaretPos() -> void
{
    if (!isEditable_ || storedText_.empty()) { return; }
    storedText_.pop_back();
    notifyTyping();
}

auto CaretTextHandler::moveCaretLeft(const uint32_t step) -> void
{
    (void)step;
    // TODO
}

auto CaretTextHandler::moveCaretRight(const uint32_t step) -> void
{
    (void)step;
    // TODO
}

auto CaretTextHandler::setFocused(const bool focused) -> void
{
    if (!isEditable_) { return; }

    if (focused)
    {
        CaretHelper::get().start();
        CaretHelper::get().setBlinkTime(blinkIntervalMs_);
        CaretHelper::get().setCaretColor(caretColor_);
    }
    else
    {
        CaretHelper::get().stop();
    }

    focused_ = focused;
}

auto CaretTextHandler::setCaretColor(const glm::vec4& color) -> void
{
    caretColor_ = color;
}

auto CaretTextHandler::setText(const std::string& text) -> void
{
    BasicTextHandler::setText(text);
    notifyTyping();
}

auto CaretTextHandler::setBlinkTime(const std::chrono::milliseconds& ms) -> void
{
    blinkIntervalMs_ = ms;
}

auto CaretTextHandler::setEditable(const bool editable) -> void
{
    isEditable_ = editable;
}

auto CaretTextHandler::getText() const -> std::string
{
    return storedText_;
}
} // namespace lav::core