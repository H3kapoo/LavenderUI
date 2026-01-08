#include "UITreeView.hpp"

#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/LayoutHandler/Calculators/PaneCalculator.hpp"
#include "src/Core/ViewModels/AbstractModel.hpp"
#include "src/Core/ViewModels/TreeModels.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::node
{
UITreeView::UITreeView(UIBaseInitData&& initData)
    : UIPane(std::move(initData))
    , model_{nullptr}
    , selectedId_(0)
    , topOfTheListIdx_{0}
    , oldTopOfTheListIdx_{-1}
    , visibleCount_{0}
    , oldVisibleCount_{-1}
    , tolerance_{2}
    , rowSize_{16}
{
    setScrollEnabled(false, true);
    setBorderColor(utils::hexToVec4("#c0cbcdff"));
    setColor(utils::hexToVec4("#c0cbcdff"));
    layoutBase_.setType(core::LayoutBase::Type::VERTICAL);
    layoutBase_.setBorder(4);
    vScroll_->getBaseLayoutData().setMargin({0, 0, 4, 0});
}

auto UITreeView::onRender(const glm::mat4& projection) -> void
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

auto UITreeView::onLayout() -> void
{
    /* Slider value needs to be reset to zero if there's no need for it anymore after an
    item has closed. */
    if (flattenedList_.size() * rowSize_ - layoutBase_.getContentBoxScale().y <= 0)
    {
        vScroll_ ? vScroll_->setScrollValue(0) : void();
    }

    glm::i64vec2 overflow{0, 0};
    overflow.y = flattenedList_.size() * rowSize_ - layoutBase_.getContentBoxScale().y;
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

auto UITreeView::calculateLayout() -> glm::i64vec2
{
    const auto& calculator = core::PaneCalculator::get();
    glm::i64vec2 overflow{0, 0};

    const auto sliderImpact = calculator.calculateSlidersScaleAndPos(this);
    calculator.calculateScaleForGenericElement(this, sliderImpact);
    calculator.calculatePositionForGenericElement(this, sliderImpact);

    overflow = calculator.calculateElementOverflow(this, sliderImpact);

    return overflow;
}

auto UITreeView::onEvent(node::UIStatePtr& state) -> void
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

auto UITreeView::resolveVisibleItems() -> void
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


    for (int32_t i = 0; i < visibleCount_; ++i)
    {
        uint64_t viewRow = topOfTheListIdx_ + i;
        if (viewRow >= flattenedList_.size()) { break; }

        auto itemObj = utils::make<UIButton>();

        // core::ModelIndex idx = model_->index(viewRow, 0, core::ModelIndex{});
        core::ModelIndex idx = flattenedList_[viewRow];

        const uint32_t depth = model_->depth(idx);
        const int32_t margin = 50.0f * depth;
        itemObj->setText(model_->data(idx) + " /\\ " + std::to_string(depth));

        /* Set private stuff on the visual object. */
        core::LayoutBase::ScaleXY scale
        {
            core::LayoutBase::Scale(130 + margin, core::LayoutBase::ScaleType::PX),
            core::LayoutBase::Scale(rowSize_, core::LayoutBase::ScaleType::PX)
        };

        itemObj->getBaseLayoutData()
            .setScale(scale)
            .setMargin({0, 0, margin, 0});
        itemObj->setColor(viewRow % 2
            ? utils::hexToVec4("#adadadff")
            : utils::hexToVec4("#e46b6bff"));
        itemObj->listenEvent<core::MouseLeftReleaseEvt>(
            [this, idx](const auto&)
            {
                core::ViewLMBRelease evt{idx};
                eventsMgr_.emitEvent<core::ViewLMBRelease>(evt);
            });

        UIBase::add(itemObj);
    }
    oldTopOfTheListIdx_ = topOfTheListIdx_;
    oldVisibleCount_ = visibleCount_;
}

auto UITreeView::computeFlatList() -> void
{
    log_.warn("start --------");
    const core::ModelIndex root{};

    auto genFlatlist = [this](auto&& self, const core::ModelIndex r) -> void
    {
        if (r.isValid())
        {
            flattenedList_.push_back(r);
        }
        for (uint32_t i = 0; i < model_->getRowCount(r); ++i)
        {
            const core::ModelIndex m = model_->index(i, 0, r);
            self(self, m);
        }
    };

    genFlatlist(genFlatlist, root);

    for (const auto& x : flattenedList_)
    {
        // log_.error("we have {} {}", x.row, x.internalPtr ? "not_root" : "root");
        log_.error("we have {} ", model_->data(x));
    }
}

auto UITreeView::setModel(const core::AbstractModelPtr model) -> void
{
    model_ = model;
    computeFlatList();
}
} // namespace lav::node
