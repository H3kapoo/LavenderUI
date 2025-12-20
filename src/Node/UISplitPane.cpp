#include "UISplitPane.hpp"

#include <algorithm>
#include <numeric>

#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LayoutHandler/Calculators/SplitPaneCalculator.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Node/Helpers/UIState.hpp"
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
    , currentDistribIdx_(0)
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
    if (elements_.size() < MIN_ELEMENTS_REQUIRED)
    {
        log_.warn("Can't work with '{}' panes! Minimum of '{}' required.",
            elements_.size(), MIN_ELEMENTS_REQUIRED);
        return;
    }
    // TODO: Check in case there less than distribution says.

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

auto UISplitPane::setSplitDistribution(std::vector<float>&& distrib) -> void
{
    const static float EPSILON = 0.001f;
    float total = std::accumulate(distrib.begin(), distrib.end(), 0.0f);
    if (std::abs(total) - 1.0f > EPSILON || 1.0f - std::abs(total) > EPSILON)
    {
        log_.error("Distribution invalid, it should sum up to '1' but got '{}'", total);
        return;
    }

    initialDistribution_ = std::move(distrib);
}

auto UISplitPane::createPane(UIPanePtr&& pane) -> void
{
    if (!checkDistribBound(__func__)) { return; }

    create<UIPane>(std::move(pane), initialDistribution_[currentDistribIdx_++], std::nullopt);
}

auto UISplitPane::createSubsplit(UISplitPanePtr&& subSplit) -> void
{
    if (!checkDistribBound(__func__)) { return; }

    create<UISplitPane>(std::move(subSplit), initialDistribution_[currentDistribIdx_++], std::nullopt);
}

auto UISplitPane::createPane(const glm::ivec2 minMax) -> UIPaneWPtr
{
    if (!checkDistribBound(__func__)) { return {}; }

    UIPanePtr uiElement = utils::make<UIPane>();
    return create(std::move(uiElement), initialDistribution_[currentDistribIdx_++], minMax);
}

auto UISplitPane::createSubsplit(const glm::ivec2 minMax) -> UISplitPaneWPtr
{
    if (!checkDistribBound(__func__)) { return {}; }

    UISplitPanePtr uiElement = utils::make<UISplitPane>();
    return create(std::move(uiElement), initialDistribution_[currentDistribIdx_++], minMax);
}

template<UISplitPaneElement T>
auto UISplitPane::create(std::shared_ptr<T>&& uiElement, const float relativeSpace,
    const std::optional<glm::ivec2> minMax) -> std::weak_ptr<T>
{
    auto& uiElementLayout = uiElement->getBaseLayoutData();

    uiElement->setColor(utils::randomRGB()); // rm later

    const core::LayoutBase::Scale scale{relativeSpace, core::LayoutBase::ScaleType::REL};
    if (layoutBase_.isHorizontal())
    {
        uiElementLayout.setScale({scale, 1.0_rel});
        if (minMax.has_value())
        {
            uiElementLayout.setMinScale({(*minMax).x, MAX_SCALE_CAP});
            uiElementLayout.setMaxScale({(*minMax).y, MAX_SCALE_CAP});
        }
    }
    else if (layoutBase_.isVertical())
    {
        uiElementLayout.setScale({1.0_rel, scale});
        if (minMax.has_value())
        {
            uiElementLayout.setMinScale({MAX_SCALE_CAP, (*minMax).x});
            uiElementLayout.setMaxScale({MAX_SCALE_CAP, (*minMax).y});
        }
    }

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

auto UISplitPane::checkDistribBound(const char* funcName) -> bool
{
    if (initialDistribution_.empty())
    {
        log_.warn("No valid split distribution set. Please set one first.");
        return false;
    }

    if (currentDistribIdx_ >= initialDistribution_.size())
    {
        log_.error("Cannot '{}' at index '{}' as it will be more than initial distribution size.",
            funcName, currentDistribIdx_);
        return false;
    }
    return true;
}

auto UISplitPane::getPaneIdx(const uint32_t idx) -> UIPaneWPtr
{
    if (idx * 2 >= elements_.size())
    {
        log_.error("No Pane has index '{}'", idx);
        return {};
    }

    return utils::as<UIPane>(elements_.at(idx * 2));
}

auto UISplitPane::getHandleIdx(const uint32_t idx) -> UIButtonWPtr
{
    if (idx * 2 + 1 >= elements_.size())
    {
        log_.error("No Handle has index '{}'", idx);
        return {};
    }

    return utils::as<UIButton>(elements_.at(idx * 2 + 1));
}

auto UISplitPane::getSplitDistribution() const -> std::vector<float>
{
    return initialDistribution_;
}
} // namespace lav::node
