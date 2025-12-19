#include "UISplitPane.hpp"

#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LayoutHandler/Calculators/SplitPaneCalculator.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Node/Helpers/UIState.hpp"
#include "src/Node/UIPane.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::node
{
UISplitPane::UISplitPane(UIBaseInitData&& initData)
    : UIBase(std::move(initData))
    , mousePos_(-1, -1)
    , wantedCursor_(std::nullopt)
    , draggedHandleId_(0)
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
    const auto& calculator = core::SplitPaneCalculator::get();
    calculator.calculateSplitPaneElements(this, draggedHandleId_, mousePos_);
}

auto UISplitPane::onEvent(node::UIStatePtr& state) -> void
{
    if (state->currentEventId == core::MouseMoveEvt::eventId)
    {
        mousePos_ = state->mousePos;
    }

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
auto UISplitPane::create(std::shared_ptr<T>&& uiElement, const float relativeSpace,
    const glm::ivec2& minMax) -> std::weak_ptr<T>
{
    auto& uiElementLayout = uiElement->getBaseLayoutData();

    uiElement->setColor(utils::randomRGB()); // rm later

    if (layoutBase_.isHorizontal())
    {
        uiElementLayout.setScale({{relativeSpace, core::LayoutBase::ScaleType::REL}, 1.0_rel});
        uiElementLayout.setMinScale({minMax.x, 99999});
        uiElementLayout.setMaxScale({minMax.y, 99999});
    }
    else if (layoutBase_.isVertical())
    {
        uiElementLayout.setScale({1.0_rel, {relativeSpace, core::LayoutBase::ScaleType::REL}});
        uiElementLayout.setMinScale({99999, minMax.x});
        uiElementLayout.setMaxScale({99999, minMax.y});
    }

    /* No need for a handle just for one uiElement. */
    if (elements_.empty())
    {
        UIBase::add(uiElement);
        return uiElement;
    }

    UIButtonPtr handle = utils::make<UIButton>();
    auto& handleLayout = handle->getBaseLayoutData();

    handle->setText(std::to_string(elements_.size()));
    handle->setColor(utils::hexToVec4("#757575ff")); // rm later

    const core::LayoutBase::Scale handleSize = 4;
    layoutBase_.isHorizontal()
        ? handleLayout.setScale({handleSize, 1.0_rel})
        : handleLayout.setScale({1.0_rel, handleSize});

    UIBase::add(handle);
    UIBase::add(uiElement);

    const uint32_t handleIdx = elements_.size() - 2;
    handle->listenEvent<core::MouseDragEvt>([this, handleIdx](const auto&)
            {
                draggedHandleId_ = handleIdx;
            });
    handle->listenEvent<core::MouseLeftReleaseEvt>(
            [this](const auto&)
            {
                draggedHandleId_ = 0;
                wantedCursor_ = lav::Cursor::ARROW;
            });
    handle->listenEvent<core::MouseEnterEvt>(
            [this](const auto&)
            {
                wantedCursor_ = layoutBase_.isHorizontal()
                    ? lav::Cursor::HRESIZE : lav::Cursor::VRESIZE;
            });
    handle->listenEvent<core::MouseExitEvt>(
            [this](const auto&)
            {
                if (draggedHandleId_) { return; }
                wantedCursor_ = lav::Cursor::ARROW;
            });

    return uiElement;
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
