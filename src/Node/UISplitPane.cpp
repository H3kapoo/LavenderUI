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
    , needsRelativeScaleCalculation_(true)
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
    /*
        At the start or on element removal/addition in runtime we need to calculate what the
        relative scale of the elements shall be from their already set fractional scale.
    */
    if (needsRelativeScaleCalculation_)
    {
        calculateRelativeScaleFromFractionalScale();
        needsRelativeScaleCalculation_ = false;
        isRuntime_ = true;
    }

    const auto& calculator = core::SplitPaneCalculator::get();
    calculator.calculateSplitPaneElements(this, draggedHandleId_, mousePos_);
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

template<UISplitPaneElement T>
auto UISplitPane::create(std::shared_ptr<T>&& uiElement, const float fracPart,
    const glm::ivec2& minMax) -> std::weak_ptr<T>
{
    /*
        In runtime creation of new elements, we needs to translate all current elements back to
        fractional part so we can calculate a new accumulated fractional scale and a new relative scale
        scale for the elements.
    */
    if (isRuntime_)
    {
        needsRelativeScaleCalculation_ = true;
        calculateFractionalScaleFromRelativeScale();
    }

    auto& uiElementLayout = uiElement->getBaseLayoutData();

    uiElement->setColor(utils::randomRGB()); // rm later

    const core::LayoutBase::Scale scale{fracPart, core::LayoutBase::ScaleType::REL};
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

    /* Add the fractional part of the new element to the accumulator. */
    accumulatedFrationalParts_ += fracPart;

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

auto UISplitPane::removePaneIdx(const uint32_t idx) -> void
{
    /* Can't have less than 1 pane. */
    if (elements_.size() <= 1) { return; }

    UIBase::remove({elements_.at(idx), elements_.at(idx + 1)});

    /*
        In runtime deletion we need to recalculate the accumulated fractional part since now it is
        invalid given the removed element. Also a new fractional scale needs to be calculated
        for the remaining elements based on the new accumulated fractional part.
    */
    accumulateFractionalPartsOfElements();
    if (isRuntime_)
    {
        needsRelativeScaleCalculation_ = true;
        calculateFractionalScaleFromRelativeScale();
    }
    else
    {
        sizeOfOneFrac_ = 1.0f / accumulatedFrationalParts_;
    }
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

auto UISplitPane::calculateRelativeScaleFromFractionalScale() -> void
{
    /* Calculate the relative scale of a single fractional part. */
    sizeOfOneFrac_ = 1.0f / accumulatedFrationalParts_;

    /*
        Each element's currently set fractional scale will be translated into a relative scale.
        Example:
            EL_1 => 4_fr
            EL_2 => 2_fr
            EL_3 => 4_fr
        Then one fractional part is: 1 / (4 + 2 + 4) = 1 / 10 = 0.1f
        Which gives us that the elements relative scale shall be:
            EL_1 => 4 * 0.1f => 0.4f
            EL_2 => 2 * 0.1f => 0.2f
            EL_3 => 4 * 0.1f => 0.4f
        They all add up to 1.0f perfectly, which is what we want.

        Note: For this to work, elements must have their scale set as fractional part when this
            function is invoked. Aka runtime only.
    */
    for (const auto& element : getElements())
    {
        if (element->getTypeId() == UIButton::typeId) { continue; }

        const auto es = element->getBaseLayoutData().getScale();
        if (layoutBase_.isHorizontal())
        {
            element->getBaseLayoutData().setScale({es.x.val * sizeOfOneFrac_, 1.0_rel});
        }
        else if (layoutBase_.isVertical())
        {
            element->getBaseLayoutData().setScale({1.0_rel, es.y.val * sizeOfOneFrac_});
        }
    }
}

auto UISplitPane::calculateFractionalScaleFromRelativeScale() -> void
{
    /*
        Translate the elements scale from relative back to fractional.
        Example:
            EL_1 => 0.2f
            EL_2 => 0.4f
            EL_3 => 0.4f
        Knowing that one fractional part is 0.1f (calculated and cached earlier) we can
        get back the fractional part for each element:
            EL_1 => 0.2f / 0.1f => 2_fr
            EL_2 => 0.4f / 0.1f => 4_fr
            EL_3 => 0.4f / 0.1f => 4_fr
        The accumulated fractional part is: 2 + 4 + 4 = 8

        Note: For this to work, the elements scale need to be already relative, aka the sum of the
            scale of all pane elements on one axis needs to add up to 1.0f. Aka runtime only.
    */
    for (const auto& element : getElements())
    {
        if (element->getTypeId() == UIButton::typeId) { continue; }

        const auto es = element->getBaseLayoutData().getScale();
        if (layoutBase_.isHorizontal())
        {
            element->getBaseLayoutData().setScale({es.x.val / sizeOfOneFrac_, 1.0_rel});
        }
        else if (layoutBase_.isVertical())
        {
            element->getBaseLayoutData().setScale({1.0_rel, es.y.val / sizeOfOneFrac_});
        }
    }
}

auto UISplitPane::accumulateFractionalPartsOfElements() -> void
{
    /*
        Compute the accumulated fractional part of each active element. This doesn't change
        the current scale of the elements.
        Example:
            EL_1 => 0.2f
            EL_2 => 0.4f
            EL_3 => 0.4f
        Knowing that one fractional part is 0.1f (calculated and cached earlier) we can
        get back the fractional part for each element:
            EL_1 => 0.2f / 0.1f => 2_fr
            EL_2 => 0.4f / 0.1f => 4_fr
            EL_3 => 0.4f / 0.1f => 4_fr
        The accumulated fractional part is: 2 + 4 + 4 = 8

        Note: This works both at runtime and at startup. However at startup we don't need to do
            any relative -> fractional transformations.
    */
    accumulatedFrationalParts_ = 0;
    for (const auto& element : getElements())
    {
        if (element->getTypeId() == UIButton::typeId) { continue; }

        const auto es = element->getBaseLayoutData().getScale();
        if (layoutBase_.isHorizontal())
        {
            accumulatedFrationalParts_ += isRuntime_ ? es.x.val / sizeOfOneFrac_ : es.x.val;
        }
        else if (layoutBase_.isVertical())
        {
            accumulatedFrationalParts_ += isRuntime_ ? es.y.val / sizeOfOneFrac_ : es.y.val;
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
    if (idx * 2 >= elements_.size()) { return {}; }
    return utils::as<UIPane>(elements_.at(idx * 2));
}

auto UISplitPane::getHandleIdx(const uint32_t idx) -> UIButtonWPtr
{
    if (idx * 2 + 1>= elements_.size()) { return {}; }
    return utils::as<UIButton>(elements_.at(idx * 2 + 1));
}
} // namespace lav::node
