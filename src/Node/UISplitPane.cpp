#include "UISplitPane.hpp"

#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LayoutHandler/Calculators/SplitPaneCalculator.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Node/Helpers/UIState.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Node/UIPane.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::node
{
const uint32_t UISplitPane::MIN_ELEMENTS_REQUIRED = 2u;
const uint32_t UISplitPane::NO_HANDLE_ACQUIRED = 0u;
const uint32_t UISplitPane::MAX_SCALE_CAP = 99'999u;
const uint32_t UISplitPane::HANDLE_DEFAULT_SIZE = 4u;
const glm::vec4 UISplitPane::HANDLE_DEFAULT_COLOR = utils::hexToVec4("#757575ff");

UISplitPane::UISplitPane(UIBaseInitData&& initData)
    : UIBase(std::move(initData))
    , mousePos_(-1, -1)
    , wantedCursor_(std::nullopt)
    , draggedHandleId_(0)
    , accumulatedFrationalParts_(0)
    , sizeOfOneFrac_(0)
    , needsFractionalPartApplication_(true)
    , isRuntime_(false)
{}

auto UISplitPane::onRender(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", getColor());
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    shader_.uploadVec4f("uBorderRadii", layoutBase_.getBorderRadius());
    shader_.uploadVec4f("uBorderColor", getBorderColor());
    shader_.uploadInt("uUseTexture", 0);
    core::GPUBinder::get().renderBoundQuad();
}

auto UISplitPane::onLayout() -> void
{
    if (needsFractionalPartApplication_)
    {
        calculateAndApplyFractionalScale();
        isRuntime_ = true;
    }

    const auto& calculator = core::SplitPaneCalculator::get();
    calculator.calculateSplitPaneElements(this, draggedHandleId_, mousePos_);
}

auto UISplitPane::calculateAndApplyFractionalScale() -> void
{
    sizeOfOneFrac_ = 1.0f / accumulatedFrationalParts_;
    log_.error("acc is {}", accumulatedFrationalParts_);
    log_.error("one equal part is {}", sizeOfOneFrac_);

    for (const auto& p : getElements())
    {
        if (p->getTypeId() == UIButton::typeId) { continue; }
        auto cs = p->getBaseLayoutData().getScale();

        if (layoutBase_.isHorizontal())
        {
            cs.x.val *= sizeOfOneFrac_;
            p->getBaseLayoutData().setScale({cs.x, 1.0_rel});
            log_.warn("new cs.x {}", cs.x.val);
        }
        else if (layoutBase_.isVertical())
        {
            cs.y.val *= sizeOfOneFrac_;
            p->getBaseLayoutData().setScale({1.0_rel, cs.y});
            log_.warn("new cs.y {}", cs.y.val);
        }
    }

    needsFractionalPartApplication_ = false;
}

auto UISplitPane::accumulateFractionalPartsOfElements() -> void
{
    accumulatedFrationalParts_ = 0;
    for (const auto& p : getElements())
    {
        if (p->getTypeId() == UIButton::typeId) { continue; }
        auto cs = p->getBaseLayoutData().getScale();

        if (layoutBase_.isHorizontal())
        {
            cs.x.val /= sizeOfOneFrac_;
            p->getBaseLayoutData().setScale({cs.x, 1.0_rel});

            log_.warn("prev cs.x {}", cs.x.val);
            accumulatedFrationalParts_ += cs.x.val;
        }
        else if (layoutBase_.isVertical())
        {
            cs.y.val /= sizeOfOneFrac_;
            p->getBaseLayoutData().setScale({1.0_rel, cs.y});

            log_.warn("prev cs.y {}", cs.y.val);
            accumulatedFrationalParts_ += cs.y.val;
        }
    }
}

auto UISplitPane::convertRelativeScaleBackToFractional() -> void
{
    for (const auto& p : getElements())
    {
        if (p->getTypeId() == UIButton::typeId) { continue; }
        auto cs = p->getBaseLayoutData().getScale();

        if (layoutBase_.isHorizontal())
        {
            cs.x.val /= sizeOfOneFrac_;
            p->getBaseLayoutData().setScale({cs.x, 1.0_rel});

            log_.warn("prev cs.x {}", cs.x.val);
        }
        else if (layoutBase_.isVertical())
        {
            cs.y.val /= sizeOfOneFrac_;
            p->getBaseLayoutData().setScale({1.0_rel, cs.y});

            log_.warn("prev cs.y {}", cs.y.val);
        }
    }
}

auto UISplitPane::onEvent(node::UIStatePtr& state) -> void
{
    if (state->currentEventId == core::MouseMoveEvt::eventId)
    {
        mousePos_ = state->mousePos;
    }

    handleSpecificEventsOnHandles(state);

    /* Only update the cursor after all the children have processed the event. */
    if (wantedCursor_.has_value())
    {
        state->wantedCursorType = wantedCursor_;
        wantedCursor_.reset();
    }
}

auto UISplitPane::createPane(UIPanePtr&& pane, const float relativeSpace, const glm::ivec2& minMax) -> void
{
    create<UIPane>(std::move(pane), relativeSpace, minMax);
}

auto UISplitPane::createSubsplit(UISplitPanePtr&& subSplit, const float relativeSpace,
    const glm::ivec2& minMax) -> void
{
    create<UISplitPane>(std::move(subSplit), relativeSpace, minMax);
}

auto UISplitPane::createPane(const float relativeSpace, const glm::ivec2& minMax) -> UIPaneWPtr
{
    UIPanePtr uiElement = utils::make<UIPane>();
    return create(std::move(uiElement), relativeSpace, minMax);
}

auto UISplitPane::createSubsplit(const float relativeSpace, const glm::ivec2& minMax) -> UISplitPaneWPtr
{
    UISplitPanePtr uiElement = utils::make<UISplitPane>();
    return create(std::move(uiElement), relativeSpace, minMax);
}

auto UISplitPane::removePaneIdx(const uint32_t idx) -> void
{
    if (elements_.size() <= 1) { return; }

    UIBase::remove({elements_.at(0), elements_.at(1)});

    if (isRuntime_)
    {
        needsFractionalPartApplication_ = true;
        accumulateFractionalPartsOfElements();
    }
}

template<UISplitPaneElement T>
auto UISplitPane::create(std::shared_ptr<T>&& uiElement, const float relativeSpace,
    const glm::ivec2& minMax) -> std::weak_ptr<T>
{
    needsFractionalPartApplication_ = true;
    log_.warn("setting done to false");

    log_.warn("-----------");
    if (isRuntime_)
    {
        convertRelativeScaleBackToFractional();
        // accumulateFractionalPartsOfElements();
    }

    auto& uiElementLayout = uiElement->getBaseLayoutData();

    uiElement->setColor(utils::randomRGB()); // rm later

    const core::LayoutBase::Scale scale{relativeSpace, core::LayoutBase::ScaleType::REL};
    if (layoutBase_.isHorizontal())
    {
        uiElementLayout.setScale({scale, 1.0_rel});
        uiElementLayout.setMinScale({minMax.x, MAX_SCALE_CAP});
        uiElementLayout.setMaxScale({minMax.y, MAX_SCALE_CAP});
    }
    else if (layoutBase_.isVertical())
    {
        uiElementLayout.setScale({1.0_rel, scale});
        uiElementLayout.setMinScale({MAX_SCALE_CAP, minMax.x});
        uiElementLayout.setMaxScale({MAX_SCALE_CAP, minMax.y});
    }

    accumulatedFrationalParts_ += relativeSpace;

    /* No need for a handle just for one uiElement. */
    if (elements_.empty())
    {
        UIBase::add(uiElement);
        return uiElement;
    }

    UIButtonPtr handle = utils::make<UIButton>();

    // handle->setText(std::to_string(elements_.size()));
    handle->setColor(HANDLE_DEFAULT_COLOR);

    auto& handleLayout = handle->getBaseLayoutData();
    layoutBase_.isHorizontal()
        ? handleLayout.setScale({(float)HANDLE_DEFAULT_SIZE, 1.0_rel})
        : handleLayout.setScale({1.0_rel, (float)HANDLE_DEFAULT_SIZE});

    UIBase::add(handle);
    UIBase::add(uiElement);

    return uiElement;
}

auto UISplitPane::handleSpecificEventsOnHandles(node::UIStatePtr& state) -> void
{
    if (draggedHandleId_ == NO_HANDLE_ACQUIRED
        && state->currentEventId == core::MouseMoveEvt::eventId
        && state->prevEventId == core::MouseDragEvt::eventId)
    {
        if (const auto handleIdx = getHandleIdxBasedOnId(state->clickedId); handleIdx.has_value())
        {
            draggedHandleId_ = handleIdx.value();
        }
    }
    else if (draggedHandleId_ != NO_HANDLE_ACQUIRED
        && state->currentEventId == core::MouseButtonEvt::eventId
        && state->prevEventId == core::MouseLeftReleaseEvt::eventId)
    {
        if (const auto handleIdx = getHandleIdxBasedOnId(state->selectedId); handleIdx.has_value())
        {
            draggedHandleId_ = NO_HANDLE_ACQUIRED;
            wantedCursor_ = lav::Cursor::ARROW;
        }
    }
    /* MouseExitEvt is used here to also identify MouseEnterEvt as they are one right after another guaranteed. */
    else if (state->currentEventId == core::MouseMoveEvt::eventId
        && state->prevEventId == core::MouseExitEvt::eventId)
    {
        if (auto handleIdx = getHandleIdxBasedOnId(state->hoveredId); handleIdx.has_value())
        {
            wantedCursor_ = layoutBase_.isHorizontal() ? lav::Cursor::HRESIZE : lav::Cursor::VRESIZE;
        }
        else if (handleIdx = getHandleIdxBasedOnId(state->prevHoveredId); handleIdx.has_value())
        {
            if (draggedHandleId_) { return; }
            wantedCursor_ = lav::Cursor::ARROW;
        }
    }
}

auto UISplitPane::getHandleIdxBasedOnId(const uint32_t id) -> std::optional<uint32_t>
{
    for (uint32_t handleIdx = 0; handleIdx < getElements().size(); ++handleIdx)
    {
        const auto handleEl = elements_[handleIdx];
        if (handleEl->getTypeId() == UIButton::typeId && id == handleEl->getId())
        {
            return handleIdx;
        }
    }

    return std::nullopt;
}

auto UISplitPane::getPaneIdx(const uint32_t idx) -> UIPaneWPtr
{
    //TODO: Add constraints
    return utils::as<UIPane>(elements_.at(idx * 2));
}

auto UISplitPane::getHandleIdx(const uint32_t idx) -> UIButtonWPtr
{
    //TODO: Add constraints
    return utils::as<UIButton>(elements_.at(idx * 2 + 1));
}
} // namespace lav::node
