#include "include/LavenderUI/Core/TextHandler/TextHandler.hpp"

#include <chrono>

#include "include/LavenderUI/Core/TextHandler/CaretHelper.hpp"
#include "include/LavenderUI/Core/TextHandler/SimpleText.hpp"

namespace lav::core
{
TextHandler::TextHandler(const fs::path& vertShaderPath, const fs::path& fragShaderPath)
    : SimpleText(vertShaderPath, fragShaderPath)
    , log_("TextHandler")
    , blinkIntervalMs_(std::chrono::milliseconds(500))
    , focused_(false)
    , isEditable_(false)
{}

auto TextHandler::render(const glm::mat4& projection) -> void
{
    SimpleText::render(projection);

    /* Check if caret needs to be displayed. */
    if (!isEditable_ || !focused_) { return; }

    glm::ivec2 caretPos_;
    caretPos_.x = startPos_.x + computeMaxSize().x;
    caretPos_.y = startPos_.y + computeMaxSize().y / 2.0f - 20 / 2.0f;
    CaretHelper::get().setCaretPos(caretPos_);
    CaretHelper::get().render(projection);
}

auto TextHandler::simulateMouseAt(const glm::ivec2 pos) -> void
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

auto TextHandler::notifyTyping() -> void
{
    CaretHelper::get().requestKeepAlive();
}

auto TextHandler::appendAtCaretPos(const char chr) -> void
{
    if (!isEditable_) { return; }
    storedText_ += chr;
    notifyTyping();
}

auto TextHandler::removeAtCaretPos() -> void
{
    if (!isEditable_) { return; }
    storedText_.pop_back();
    notifyTyping();
}

auto TextHandler::moveCaretLeft(const uint32_t step) -> void
{
    (void)step;
    // TODO
}

auto TextHandler::moveCaretRight(const uint32_t step) -> void
{
    (void)step;
    // TODO
}

auto TextHandler::setFocused(const bool focused) -> void
{
    if (!isEditable_) { return; }

    if (focused)
    {
        CaretHelper::get().start();
        CaretHelper::get().setBlinkTime(blinkIntervalMs_);
    }
    else
    {
        CaretHelper::get().stop();
    }

    focused_ = focused;
}

auto TextHandler::setText(const std::string& text) -> void
{
    SimpleText::setText(text);
    notifyTyping();
}

auto TextHandler::setBlinkTime(const std::chrono::milliseconds& ms) -> void
{
    blinkIntervalMs_ = ms;
}

auto TextHandler::setEditable(const bool editable) -> void
{
    isEditable_ = editable;
}

auto TextHandler::getText() const -> std::string
{
    return storedText_;
}
} // namespace lav::core