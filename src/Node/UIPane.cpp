#include "include/LavenderUI/Node/UIPane.hpp"

#include "include/LavenderUI/Core/Binders/GPUBinder.hpp"
#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"
#include "include/LavenderUI/Core/LayoutHandler/Calculators/PaneCalculator.hpp"
#include "include/LavenderUI/Core/ResourceHandler/Shader.hpp"
#include "include/LavenderUI/Core/State/UIState.hpp"
#include "include/LavenderUI/Utils/Misc.hpp"

namespace lav::node
{
UIPane::UIPane(UIBaseInitData&& initData)
    : UIBase(std::move(initData))
    , hScroll_(nullptr)
    , vScroll_(nullptr)
{
    layoutBase_.setScale({200_px, 50_px});
}

auto UIPane::onRender(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", baseColor_);
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    shader_.uploadVec4f("uBorderRadii", layoutBase_.getBorderRadius());
    shader_.uploadVec4f("uBorderColor", borderColor_);
    shader_.uploadInt("uUseTexture", 0);
    core::GPUBinder::get().renderBoundQuad();
}

auto UIPane::onLayout() -> void
{
    /* Calculate the layout and do it again if the layout became invalid. */
    if (setInternalScrollOverflow(calculateLayout()))
    {
        setInternalScrollOverflow(calculateLayout());
    }

    const auto& calculator = core::PaneCalculator::get();
    calculator.calculateElementsOffsetDueToScroll(this,
    {
        hScroll_ ? hScroll_->getScrollValue() : 0,
        vScroll_ ? vScroll_->getScrollValue() : 0
    });
}

auto UIPane::onEvent(core::UIStatePtr& state) -> void
{
    const auto eId = state->currentEventId;
    if (eId == core::MouseButtonEvt::eventId)
    {
        core::MouseButtonEvt e{state->mouseButton, state->mouseAction};
        eventsMgr_.emitEvent<core::MouseButtonEvt>(e);
    }
    else if (state->currentEventId == core::MouseMoveEvt::eventId)
    {
        if (layoutBase_.isPointInsideView(state->mousePos))
        {
            state->closestScrollId = getClosestScrollbar(state->mousePos);
        }
    }
    else if (state->currentEventId == core::MouseEnterEvt::eventId)
    {
        core::MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseEnterEvt>(e);
    }
    else if (state->currentEventId == core::MouseExitEvt::eventId)
    {
        core::MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseExitEvt>(e);
    }
}

auto UIPane::calculateLayout() -> glm::ivec2
{
    const auto& calculator = core::PaneCalculator::get();
    glm::ivec2 overflow{0, 0};

    const auto sliderImpact = calculator.calculateSlidersScaleAndPos(this);
    calculator.calculateScaleForGenericElement(this, sliderImpact);
    calculator.calculatePositionForGenericElement(this, sliderImpact);

    overflow = calculator.calculateElementOverflow(this, sliderImpact);
    calculator.calculateAlignmentForElements(this, overflow);

    return overflow;
}

auto UIPane::setInternalScrollOverflow(const glm::ivec2 overflow) -> bool
{
    bool shouldRedoLayout{false};
    shouldRedoLayout |= addAndSetIfNeeded(hScroll_, overflow.x);
    shouldRedoLayout |= addAndSetIfNeeded(vScroll_, overflow.y);
    shouldRedoLayout |= removeAndSetIfNeeded(hScroll_, overflow.x);
    shouldRedoLayout |= removeAndSetIfNeeded(vScroll_, overflow.y);

    return shouldRedoLayout;
}

auto UIPane::addAndSetIfNeeded(const UIScrollPtr scrollNode, const int32_t overflow) -> bool
{
    if (overflow <= 0 || !scrollNode) { return false; }

    if (scrollNode->isParented())
    {
        scrollNode->setScrollTo(overflow);
    }
    else
    {
        UIBase::add(scrollNode);
        scrollNode->setScrollTo(overflow);
        return true;
    }

    return false;
}

auto UIPane::removeAndSetIfNeeded(const UIScrollPtr scrollNode, const int32_t overflow) -> bool
{
    if (overflow > 0 || !scrollNode) { return false; }

    if (scrollNode->isParented())
    {
        UIBase::remove(scrollNode);
        scrollNode->setScrollTo(0);
        return true;
    }
    else
    {
        scrollNode->setScrollTo(0);
    }

    return false;
}

auto UIPane::getClosestScrollbar(const glm::ivec2 pMouse) const -> uint32_t
{
    uint32_t closestScrollbarId{core::NOTHING};

    /* When outside the UIScroll, prioritize vertical bar no matter where the mouse is. */
    //TODO: Actually yes, take into consideration if we are inside the pane that has the scroll enabled
    if (hScroll_ && hScroll_->isParented())
    {
        closestScrollbarId = hScroll_->getId();
    }

    if (vScroll_ && vScroll_->isParented())
    {
        closestScrollbarId = vScroll_->getId();
    }

    /* When inside the UIScroll, prioritize whatever UIScroll the mouse happens to be on. */
    if (vScroll_ && vScroll_->isParented()
        && vScroll_->getBaseLayoutData().isPointInsideView(pMouse))
    {
        closestScrollbarId = vScroll_->getId();
    }
    else if (hScroll_ && hScroll_->isParented()
        && hScroll_->getBaseLayoutData().isPointInsideView(pMouse))
    {
        closestScrollbarId = hScroll_->getId();
    }

    return closestScrollbarId;
}

auto UIPane::setScrollEnabled(const bool enableHorizontal, const bool enableVertical) -> UIPane&
{
    if (enableHorizontal)
    {
        hScroll_ = utils::make<UIScroll>();
        hScroll_->getBaseLayoutData().setType(core::LayoutBase::Type::HORIZONTAL)
            .setScale({1.0_rel, 20_px});
    }
    else
    {
        hScroll_.reset();
    }

    if (enableVertical)
    {
        vScroll_ = utils::make<UIScroll>();
        vScroll_->setInvertAxis(true);
        vScroll_->getBaseLayoutData().setType(core::LayoutBase::Type::VERTICAL)
            .setScale({20_px, 1.0_rel});
    }
    else
    {
        vScroll_.reset();
    }

    return *this;
}

auto UIPane::setScrollSensitivity(const float value) -> UIPane&
{
    vScroll_ ? vScroll_->setScrollSensitivity(value) : void();
    hScroll_ ? hScroll_->setScrollSensitivity(value) : void();
    return *this;
}

auto UIPane::isVerticalOverflow() const -> bool { return vScroll_ ? true : false; }

auto UIPane::isHorizontalOverflow() const -> bool { return hScroll_ ? true : false; }

auto UIPane::getHorizontalScroll() const -> UIScrollWPtr { return hScroll_; }

auto UIPane::getVerticalScroll() const -> UIScrollWPtr { return vScroll_; }
} // namespace lav::node
