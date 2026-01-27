#include "include/LavenderUI/Node/UILineEdit.hpp"

#include <chrono>
#include <optional>

#include "include/LavenderUI/Core/Binders/GPUBinder.hpp"
#include "include/LavenderUI/Core/Binders/WindowBinder.hpp"
#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"
#include "include/LavenderUI/Core/LayoutHandler/LayoutBase.hpp"
#include "include/LavenderUI/Core/TextHandler/TextHandler.hpp"
#include "include/LavenderUI/Utils/Misc.hpp"

namespace lav::node
{
UILineEdit::UILineEdit(UIBaseInitData&& data)
    : UIBase(std::move(data))
    , textHandler_("assets/shaders/basicTextVert.glsl", "assets/shaders/basicTextFrag.glsl")
    , placeholderText_()
    , overrideColor_(std::nullopt)
    , numericOnly_(false)
{
    textHandler_.setBlinkTime(std::chrono::milliseconds(500));
    textHandler_.setEditable(true);
    textHandler_.setTextColor(utils::hexToVec4("#333333ff"));
    textHandler_.setCaretColor(utils::hexToVec4("#333333ff"));
    layoutBase_.setScale({200_px, 50_px});
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
    textHandler_.render(projection);
}

auto UILineEdit::onLayout() -> void
{
    const glm::vec2 p = layoutBase_.getComputedPos() + layoutBase_.getComputedScale() / 2.0f
        - textHandler_.computeMaxSize() / 2.0f;

    textHandler_.setAnchorPos(p);
    textHandler_.setStartZIndex(layoutBase_.getZIndex());
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
            textHandler_.appendAtCaretPos(recentCp);

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
            textHandler_.removeAtCaretPos();

            core::TextChangedEvt e{getText()};
            eventsMgr_.emitEvent<core::TextChangedEvt>(e);
        }
        else if (state->keyRecent.value() == lav::Key::LEFT)
        {
            textHandler_.moveCaretLeft();
        }
        else if (state->keyRecent.value() == lav::Key::RIGHT)
        {
            textHandler_.moveCaretRight();
        }
    }
    else if (eId == core::FocusGainEvt::eventId)
    {
        textHandler_.setFocused(true);

        layoutBase_.setBorder({1});
        setBorderColor(utils::hexToVec4("#505350ff"));

        core::FocusGainEvt e;
        eventsMgr_.emitEvent<core::FocusGainEvt>(e);
    }
    else if (eId == core::FocusLostEvt::eventId)
    {
        textHandler_.setFocused(false);

        layoutBase_.setBorder({0});
        core::FocusLostEvt e;
        eventsMgr_.emitEvent<core::FocusLostEvt>(e);
    }
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

auto UILineEdit::enableNumbericOnly(const bool enable) -> void
{
    numericOnly_ = enable;
}

auto UILineEdit::setText(const std::string& text) -> void
{
    textHandler_.setText(text);
}

auto UILineEdit::setFont(const std::filesystem::path& fontPath) -> void
{
    textHandler_.setFont(fontPath);
}

auto UILineEdit::setTextColor(const glm::vec4& color) -> void
{
    textHandler_.setTextColor(color);
    textHandler_.setCaretColor(color);
}

auto UILineEdit::getText() const -> std::string
{
    return textHandler_.getText();
}
} // namespace src::uinodes
