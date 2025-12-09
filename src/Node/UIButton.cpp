#include "UIButton.hpp"

#include <optional>

#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Utils/Misc.hpp"
#include "src/Core/LayoutHandler/BaseCalculator.hpp"
#include "src/Core/Binders/GPUBinder.hpp"

namespace lav::node
{
UIButton::UIButton(UIBaseInitData&& data) : UIBase(std::move(data))
{
    layoutBase_.setScale({100_px, 36_px});
    label_->getBaseLayoutData().setScale({1_fill, 1_fill});
    label_->setColor(utils::hexToVec4("#ffffff00"));
    UIBase::add(label_);
}

auto UIButton::onRender(const glm::mat4& projection) -> void
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
}

auto UIButton::onLayout() -> void
{
    const auto& calculator = core::BaseCalculator::get();
    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForGenericElement(this);
}

auto UIButton::onEvent(UIStatePtr& state) -> void
{
    if (!isBtnEnabled_) { return; }

    const auto eId = state->currentEventId;
    if (eId == core::MouseLeftClickEvt::eventId)
    {
        overrideColor_ = clickedColor_;
        core::MouseLeftClickEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseLeftClickEvt>(e);
    }
    else if (eId == core::MouseLeftReleaseEvt::eventId)
    {
        if (state->hoveredId == getId()) { overrideColor_ = hoveredColor_; }
        else { overrideColor_.reset(); }

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
        if (state->clickedId != getId()) { overrideColor_ = hoveredColor_; }

        core::MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseEnterEvt>(e);
    }
    else if (eId == core::MouseExitEvt::eventId)
    {
        if (state->clickedId == getId()) { overrideColor_ = clickedColor_; }
        else { overrideColor_.reset(); }

        core::MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseExitEvt>(e);
    }
}

auto UIButton::onResetToDefault() -> void { overrideColor_.reset(); }

auto UIButton::setDisabled() -> UIButton&
{
    isBtnEnabled_ = false;
    overrideColor_ = utils::hexToVec4("#aaaaaaff");
    return *this;
}

auto UIButton::setClickedColor(const glm::vec4& color) -> UIButton& { clickedColor_ = color; return *this; }

auto UIButton::setHoveredColor(const glm::vec4& color) -> UIButton& { hoveredColor_ = color; return *this; }

auto UIButton::setEnabled() -> UIButton& { isBtnEnabled_ = true; overrideColor_.reset(); return *this; }

auto UIButton::setText(const std::string& text) -> UIButton& { label_->setText(text); return *this; }

auto UIButton::isEnabled() -> bool { return isBtnEnabled_; }

auto UIButton::getText() const -> std::string { return label_->getText(); }

auto UIButton::getColor() const -> const glm::vec4& { return baseColor_; }

auto UIButton::getBorderColor() const -> const glm::vec4& { return borderColor_; }
} // namespace src::uinodes
