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
{
    textHandler_.setBlinkTime(std::chrono::milliseconds(500));
    textHandler_.setEditable(true);
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
    // TODO: Need to collapse all the generic events into one generic callable function
    // Everything that has to do with the class logic shall be implemented separately
    const auto eId = state->currentEventId;
    if (eId == core::MouseLeftClickEvt::eventId)
    {
        core::MouseLeftClickEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseLeftClickEvt>(e);
    }
    else if (eId == core::MouseLeftReleaseEvt::eventId)
    {
        core::MouseLeftReleaseEvt e;
        eventsMgr_.emitEvent<core::MouseLeftReleaseEvt>(e);
    }
    else if (eId == core::MouseDragEvt::eventId)
    {
        core::MouseDragEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseDragEvt>(e);
    }
    else if (eId == core::MouseEnterEvt::eventId)
    {
        core::MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseEnterEvt>(e);
    }
    else if (eId == core::MouseExitEvt::eventId)
    {
        core::MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseExitEvt>(e);
    }
    else if (eId == core::CharacterEvt::eventId)
    {
        const char recentCp = static_cast<char>(state->codepointRecent.value());
        textHandler_.appendAtCaretPos(recentCp);
        core::TextChangedEvt e{getText()};
        eventsMgr_.emitEvent<core::TextChangedEvt>(e);
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

auto UILineEdit::setText(const std::string& text) -> UILineEdit&
{
    textHandler_.setText(text); return *this;
}

auto UILineEdit::setFont(const std::filesystem::path& fontPath) -> UILineEdit&
{
    textHandler_.setFont(fontPath);
    return *this;
}

auto UILineEdit::getText() const -> std::string
{
    return textHandler_.getText();
}
} // namespace src::uinodes
