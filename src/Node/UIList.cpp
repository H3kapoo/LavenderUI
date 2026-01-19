#include "include/LavenderUI/Node/UIList.hpp"

#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"
#include "include/LavenderUI/Core/LayoutHandler/LayoutBase.hpp"
#include "include/LavenderUI/Core/LayoutHandler/Calculators/PaneCalculator.hpp"
#include "include/LavenderUI/Node/UIBase.hpp"
#include "include/LavenderUI/Node/UIButton.hpp"
#include "include/LavenderUI/Core/Binders/GPUBinder.hpp"
#include "include/LavenderUI/Utils/Misc.hpp"
#include "src/Node/InternalUse/UIViewItem.hpp"

namespace lav::node
{
UIList::UIList(UIBaseInitData&& initData)
    : UIPane(std::move(initData))
    , model_{nullptr}
    , selectedId_(0)
    , tolerance_{2}
    // , rowSize_{16}
    , rowSize_{28}
    , topOfTheListIdx_{0}
    , oldTopOfTheListIdx_{-1}
    , visibleCount_{0}
    , oldVisibleCount_{-1}
{
    setScrollEnabled(false, true);
    setBorderColor(utils::hexToVec4("#c0cbcdff"));
    setColor(utils::hexToVec4("#c0cbcdff"));
    layoutBase_.setType(core::LayoutBase::Type::VERTICAL);
    layoutBase_.setBorder(4);

    vScroll_->getBaseLayoutData().setMargin({0, 0, 4, 0});
}

auto UIList::onRender(const glm::mat4& projection) -> void
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

auto UIList::onLayout() -> void
{
    /* Slider value needs to be reset to zero if there's no need for it anymore after an
    item has closed. */
    if (model_->getRowCount() * rowSize_ - layoutBase_.getContentBoxScale().y <= 0)
    {
        vScroll_ ? vScroll_->setScrollValue(0) : void();
    }

    glm::i64vec2 overflow{0, 0};
    overflow.y = model_->getRowCount() * rowSize_ - layoutBase_.getContentBoxScale().y;
    setInternalScrollOverflow(overflow);

    resolveVisibleItems();

    calculateLayout();

    const auto& calculator = core::PaneCalculator::get();
    calculator.calculateElementsOffsetDueToScroll(this,
    {
        0,
        vScroll_ ? (int64_t)vScroll_->getScrollValue() % rowSize_ : 0
    });
}

auto UIList::calculateLayout() -> glm::i64vec2
{
    const auto& calculator = core::PaneCalculator::get();
    glm::i64vec2 overflow{0, 0};

    const auto sliderImpact = calculator.calculateSlidersScaleAndPos(this);
    calculator.calculateScaleForGenericElement(this, sliderImpact);
    calculator.calculatePositionForGenericElement(this, sliderImpact);

    overflow = calculator.calculateElementOverflow(this, sliderImpact);

    return overflow;
}

auto UIList::onEvent(core::UIStatePtr& state) -> void
{
    const auto eId = state->currentEventId;
    if (eId == core::MouseMoveEvt::eventId)
    {
        if (layoutBase_.isPointInsideView(state->mousePos))
        {
            state->closestScrollId = getClosestScrollbar(state->mousePos);
        }
    }
}

auto UIList::resolveVisibleItems() -> void
{
    topOfTheListIdx_ = vScroll_ ? vScroll_->getScrollValue() / rowSize_ : 0;
    visibleCount_ = layoutBase_.getContentBoxScale().y / rowSize_ + tolerance_;
    if (topOfTheListIdx_ == oldTopOfTheListIdx_ && visibleCount_ == oldVisibleCount_)
    {
        return;
    }

    UIBase::remove([this](const auto& e)
    {
        return e->getId() != vScroll_->getId() && e->getId() != hScroll_->getId();
    });

    allocatePool();

    for (int32_t i = 0; i < visibleCount_; ++i)
    {
        uint64_t viewRow = topOfTheListIdx_ + i;
        if (viewRow >= model_->getRowCount()) { break; }

        auto itemObj = uiViewItemPool_[viewRow % uiViewItemPool_.size()];

        prepareItem(viewRow, itemObj);
        UIBase::add(itemObj);
    }

    oldTopOfTheListIdx_ = topOfTheListIdx_;
    oldVisibleCount_ = visibleCount_;
}

auto UIList::allocatePool() -> void
{
    if (uiViewItemPool_.size() >= static_cast<uint32_t>(visibleCount_)) { return; }

    for (int32_t i = 0; i < visibleCount_; ++i)
    {
        auto itemObj = utils::make<UIViewItem>();
        uiViewItemPool_.push_back(itemObj);
    }
}

auto UIList::prepareItem(const uint32_t viewRow, std::shared_ptr<UIViewItem>& item) -> void
{
    core::LayoutBase::ScaleXY scale
    {
        1_fill,
        core::LayoutBase::Scale(rowSize_, core::LayoutBase::ScaleType::PX)
    };

    core::ModelIndex idx = model_->index(viewRow, 0, core::ModelIndex{});

    auto[expandBtn, infoBtn] = item->getButtonPair();
    expandBtn->getBaseLayoutData().setScale({0_px});
    infoBtn->setText(model_->data(idx));

    item->getBaseLayoutData().setScale(scale);

    infoBtn->setColor(viewRow % 2
        ? utils::hexToVec4("#adadadff")
        : utils::hexToVec4("#e46b6bff"));
    infoBtn->listenEvent<core::MouseLeftReleaseEvt>(
        [this, idx](const auto&)
        {
            core::ViewLMBRelease evt{idx};
            eventsMgr_.emitEvent<core::ViewLMBRelease>(evt);
            /*
                Note: No need to call onLayout() here as the layout itself will not be changed,
                only values inside the elements will be changed as opposed to UITreeView in which
                elements can be removed/added and layout needs to be forcefully recomputed.
            */
        });
}

auto UIList::setModel(const core::AbstractModelPtr model) -> void
{
    model_ = model;
}

auto UIList::setRowSize(const uint32_t value) -> void
{
    rowSize_ = value;
}
} // namespace lav::node
