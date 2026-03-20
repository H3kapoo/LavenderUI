#include <LavenderUI/Node/UILineEdit.hpp>

#include <chrono>
#include <optional>

#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/Binders/WindowBinder.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/Character.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/Keyboard.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/TextChanged.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/FocusGain.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/FocusLost.hpp>
#include <LavenderUI/Core/LayoutHandler/LayoutBase.hpp>
#include <LavenderUI/Core/TextHandler/CaretHelper.hpp>
#include <LavenderUI/Core/Config.hpp>
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::node
{
UILineEdit::UILineEdit(UIBaseInitData&& data)
    : UILabel(std::move(data))
    , placeholderText_()
    , caretColor_(utils::hexToVec4("#333333ff"))
    , numericOnly_(false)
    , isEditable_(true)
    , focused_(false)
    , blinkIntervalMs_(500)
{
    layoutBase_.setScale({200_px, 50_px});
    core::CaretHelper::get().setCaretScale({1, font_->baseVerticalSep});
}

auto UILineEdit::onRender(const glm::mat4& projection) -> void
{
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", overrideColor_ ? *overrideColor_ : getColor());
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    shader_.uploadVec4f("uBorderRadii", layoutBase_.getBorderRadius());
    shader_.uploadVec4f("uBorderColor", getBorderColor());
    shader_.uploadInt("uUseTexture", 0);
    core::GPUBinder::get().renderBoundQuad();

    /* Draw the text */
    // handleText(projection);
    handleCaretRendering(projection);
}

auto UILineEdit::onLayout() -> void
{
    UILabel::onLayout();
}

auto UILineEdit::onEvent(core::UIStatePtr& state) -> void
{
    UIBase::processAndEmitGenericMouseEvents(state);

    const auto eId = state->currentEventId;
    if (eId == core::CharacterEvt::eventId)
    {
        const char recentCp = static_cast<char>(state->codepointRecent.value());
        const bool shouldAccept = performFiltering(recentCp);

        if (shouldAccept)
        {
            appendAtCaretPos(recentCp);

            core::TextChangedEvt e{getText()};
            eventsMgr_.emitEvent<core::TextChangedEvt>(e);
        }
    }
    else if (eId == core::KeyboardEvt::eventId)
    {
        if (state->keyRecent.value() == lav::Key::ENTER)
        {
            setText("");
            core::TextChangedEvt e{getText()};
            eventsMgr_.emitEvent<core::TextChangedEvt>(e);
        }
        else if (state->keyRecent.value() == lav::Key::BACKSPACE)
        {
            removeAtCaretPos();

            core::TextChangedEvt e{getText()};
            eventsMgr_.emitEvent<core::TextChangedEvt>(e);
        }
        else if (state->keyRecent.value() == lav::Key::LEFT)
        {
            // moveCaretLeft();
        }
        else if (state->keyRecent.value() == lav::Key::RIGHT)
        {
            // moveCaretRight();
        }
    }
    else if (eId == core::FocusGainEvt::eventId)
    {
        setFocused(true);

        layoutBase_.setBorder({1});
        setBorderColor(utils::hexToVec4("#505350ff"));

        core::FocusGainEvt e;
        eventsMgr_.emitEvent<core::FocusGainEvt>(e);
    }
    else if (eId == core::FocusLostEvt::eventId)
    {
        setFocused(false);

        layoutBase_.setBorder({0});
        core::FocusLostEvt e;
        eventsMgr_.emitEvent<core::FocusLostEvt>(e);
    }
}

auto UILineEdit::notifyTyping() -> void
{
    core::CaretHelper::get().requestKeepAlive();
}

auto UILineEdit::appendAtCaretPos(const char chr) -> void
{
    if (!isEditable_) { return; }
    storedText_ += chr;
    notifyTyping();
}

auto UILineEdit::removeAtCaretPos() -> void
{
    if (!isEditable_ || storedText_.empty()) { return; }
    storedText_.pop_back();
    notifyTyping();
}

auto UILineEdit::setFocused(const bool focused) -> void
{
    if (!isEditable_) { return; }

    if (focused)
    {
        core::CaretHelper::get().start();
        core::CaretHelper::get().setBlinkTime(blinkIntervalMs_);
        core::CaretHelper::get().setCaretColor(caretColor_);
        core::CaretHelper::get().setCaretScale({1, font_->baseVerticalSep});
    }
    else
    {
        core::CaretHelper::get().stop();
    }

    focused_ = focused;
}

auto UILineEdit::performFiltering(const char codepoint) -> bool
{
    bool filterPassed{false};
    const bool numericRule = codepoint >= '0' && codepoint <= '9';
    if (numericOnly_ && numericRule)
    {
        filterPassed = true;
    }
    else if (!numericOnly_)
    {
        filterPassed = true;
    }

    return filterPassed;
}

auto UILineEdit::handleCaretRendering(const glm::mat4& projection) -> void
{
    /* Check if caret needs to be displayed. */
    if (!isEditable_ || !focused_) { return; }

    glm::ivec2 caretPos_;
    // caretPos_.x = basePos_.x;
    // caretPos_.y = basePos_.y - font_->baseVerticalSep - font_->descender;
    core::CaretHelper::get().setCaretPos(caretPos_);
    core::CaretHelper::get().render(projection);
}

auto UILineEdit::setCaretColor(const glm::vec4& color) -> void
{
    caretColor_ = color;
}

auto UILineEdit::setBlinkTime(const std::chrono::milliseconds& ms) -> void
{
    blinkIntervalMs_ = ms;
}

auto UILineEdit::setEditable(const bool editable) -> void
{
    isEditable_ = editable;
}

auto UILineEdit::setNumbericOnly(const bool enable) -> void
{
    numericOnly_ = enable;
}
} // namespace src::uinodes
