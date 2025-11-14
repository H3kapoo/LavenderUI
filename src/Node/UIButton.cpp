#include "UIButton.hpp"

#include <optional>

#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Utils/Misc.hpp"
#include "src/Core/LayoutHandler/BasicCalculator.hpp"
#include "src/Core/Binders/GPUBinder.hpp"

namespace lav::node
{
using namespace core;

UIButton::UIButton(UIBaseInitData&& data) : UIBase(std::move(data))
{
    layoutBase_.setScale({100_px, 36_px});
    label_->getBaseLayoutData().setScale({1_fill, 1_fill});
    label_->setColor(utils::hexToVec4("#ffffff7d"));
    add(label_);
}

auto UIButton::render(const glm::mat4& projection) -> void
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

auto UIButton::layout() -> void
{
    const auto& calculator = core::BasicCalculator::get();
    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForGenericElement(this);
}

auto UIButton::event(UIStatePtr& state) -> void
{
    if (!isBtnEnabled_) { return; }

    using namespace core;
    const auto eId = state->currentEventId;
    if (eId == MouseLeftClickEvt::eventId)
    {
        overrideColor_ = clickedColor_;
        MouseLeftClickEvt e{state->mousePos.x, state->mousePos.y};
        return eventsMgr_.emitEvent<MouseLeftClickEvt>(e);
    }
    else if (eId == MouseLeftReleaseEvt::eventId)
    {
        if (state->hoveredId == getId()) { overrideColor_ = hoveredColor_; }
        else { overrideColor_.reset(); }

        MouseLeftReleaseEvt e;
        return eventsMgr_.emitEvent<MouseLeftReleaseEvt>(e);
    }
    else if (eId == MouseDragEvt::eventId)
    {
        MouseDragEvt e{state->mousePos.x, state->mousePos.y};
        return eventsMgr_.emitEvent<MouseDragEvt>(e);
    }
    else if (eId == MouseEnterEvt::eventId)
    {
        if (state->clickedId != getId()) { overrideColor_ = hoveredColor_; }

        MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        return eventsMgr_.emitEvent<MouseEnterEvt>(e);
    }
    else if (eId == MouseExitEvt::eventId)
    {
        if (state->clickedId == getId()) { overrideColor_ = clickedColor_; }
        else { overrideColor_.reset(); }

        MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        return eventsMgr_.emitEvent<MouseExitEvt>(e);
    }
}

auto UIButton::setClickedColor(const glm::vec4& color) -> UIButton& { clickedColor_ = color; return *this; }
auto UIButton::setHoveredColor(const glm::vec4& color) -> UIButton& { hoveredColor_ = color; return *this; }
auto UIButton::setEnabled() -> UIButton& { isBtnEnabled_ = true; overrideColor_.reset(); return *this; }
auto UIButton::setDisabled() -> UIButton&
{
    isBtnEnabled_ = false;
    overrideColor_ = utils::hexToVec4("#aaaaaaff");
    return *this;
}
auto UIButton::setText(const std::string& text) -> UIButton& { label_->setText(text); return *this; }
auto UIButton::isEnabled() -> bool { return isBtnEnabled_; }
auto UIButton::getColor() const -> const glm::vec4& { return baseColor_; }
auto UIButton::getBorderColor() const -> const glm::vec4& { return borderColor_; }
} // namespace src::uinodes
