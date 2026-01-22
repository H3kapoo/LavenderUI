#include "include/LavenderUI/Node/UILineEdit.hpp"

#include <optional>

#include "include/LavenderUI/Core/Binders/GPUBinder.hpp"
#include "include/LavenderUI/Core/Binders/WindowBinder.hpp"
#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"
#include "include/LavenderUI/Utils/Misc.hpp"

namespace lav::node
{
UILineEdit::UILineEdit(UIBaseInitData&& data)
    : UIBase(std::move(data))
    , textAttribs_()
    , placeholderText_()
    , overrideColor_(std::nullopt)
{
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
    const auto& textShader_ = textAttribs_.getShader();
    const auto& textBuffer = textAttribs_.getBuffer();
    textShader_.bind();
    textShader_.uploadVec4f("uColor", utils::hexToVec4("#141414ff"));
    textShader_.uploadMat4("uMatrixProjection", projection);
    textShader_.uploadMat4v("uModelMatrices", textBuffer.model);
    textShader_.uploadIntv("uCharIndices", textBuffer.glyphCode);
    textShader_.uploadTexture2DArray("uTextureArray", 0, textAttribs_.getFont()->textureId);
    core::GPUBinder::get().renderBoundQuadInstanced(textAttribs_.getText().size());
}

auto UILineEdit::onLayout() -> void
{
    const glm::vec2 p = layoutBase_.getComputedPos() + layoutBase_.getComputedScale() / 2.0f
        - textAttribs_.computeMaxSize() / 2.0f;
    textAttribs_.setPosition({p.x, p.y, layoutBase_.getZIndex()});
}

auto UILineEdit::onEvent(core::UIStatePtr& state) -> void
{
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
        // TODO: Technically its a uint32_t because in the future utf-8 shall be supported
        // but for now, the ASCII set is enough
        char recentCp = static_cast<char>(state->codepointRecent.value());
        textAttribs_.appendChar(recentCp);

        core::TextChangedEvt e{getText()};
        eventsMgr_.emitEvent<core::TextChangedEvt>(e);
    }
    else if (eId == core::KeyboardEvt::eventId)
    {
        char val = static_cast<char>(state->keyRecent.value());
        if (state->keyRecent.value() == lav::Key::ENTER)
        {
            setText("");
            core::TextChangedEvt e{getText()};
            eventsMgr_.emitEvent<core::TextChangedEvt>(e);
        }
        else if (state->keyRecent.value() == lav::Key::BACKSPACE)
        {
            textAttribs_.eraseChar();
            core::TextChangedEvt e{getText()};
            eventsMgr_.emitEvent<core::TextChangedEvt>(e);
        }
    }
    else if (eId == core::FocusGainEvt::eventId)
    {
        layoutBase_.setBorder({1});
        setBorderColor(utils::hexToVec4("#505350ff"));

        core::FocusGainEvt e;
        eventsMgr_.emitEvent<core::FocusGainEvt>(e);
    }
    else if (eId == core::FocusLostEvt::eventId)
    {
        layoutBase_.setBorder({0});
        core::FocusLostEvt e;
        eventsMgr_.emitEvent<core::FocusLostEvt>(e);
    }
}

auto UILineEdit::setText(const std::string& text) -> UILineEdit& { textAttribs_.setText(text); return *this; }

auto UILineEdit::setFont(const std::filesystem::path& fontPath) -> void { (void)fontPath; }

auto UILineEdit::getText() const -> std::string { return textAttribs_.getText(); }
} // namespace src::uinodes
