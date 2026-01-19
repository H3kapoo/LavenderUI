#include "include/LavenderUI/Node/UITreeView.hpp"

#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"
#include "include/LavenderUI/Core/LayoutHandler/LayoutBase.hpp"
#include "include/LavenderUI/Core/LayoutHandler/Calculators/PaneCalculator.hpp"
#include "include/LavenderUI/Core/ViewModels/AbstractModel.hpp"
#include "include/LavenderUI/Core/Binders/GPUBinder.hpp"
#include "include/LavenderUI/Utils/Misc.hpp"
#include "src/Node/InternalUse/UIViewItem.hpp"

namespace lav::node
{
UITreeView::UITreeView(UIBaseInitData&& initData)
    : UIPane(std::move(initData))
    , model_{nullptr}
    , selectedId_(0)
    , tolerance_{2}
    , rowSize_{22}
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

auto UITreeView::onEvent(core::UIStatePtr& state) -> void
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
    // TODO: If we have enough space to accomodate new items as is, then do not remove
    // the items that are already in place. This minimizes the times we need to do
    // this complex op
    // Not sure if this goes the other way around, if there are less items to show than
    // current visible count.
    // This is true for RecycleList as well
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
        if (viewRow >= flattenedList_.size()) { break; }

        auto itemObj = uiItemPool_[viewRow % uiItemPool_.size()];
        prepareItem(viewRow, itemObj);

        UIBase::add(itemObj);
    }
    oldTopOfTheListIdx_ = topOfTheListIdx_;
    oldVisibleCount_ = visibleCount_;
}

auto UITreeView::computeFlatList() -> void
{
    /*
        The tree will be flattened into a normal linear vector so that it is easier to
        index the elements using operator[] when it comes to using only a part of the tree
        for rendering/layout purposes. The flattened list will only contain items that are
        toggled open.

        depth 0:      a       b
                    / | \      \
        depth 1:   c  d  e      f
                      \
        depth 2:       g

        Flattened list will be (assuming all open): a c d g e b f
        -- a           -- a
           -- c        -- c
           -- d        -- d
              -- g     -- g
           -- e        -- e
        -- b           -- b
           -- f        -- f

        We will use pre-order traversal to populate the flat list.
    */

    // TODO: If Node A has child Node B with children and we collapse node A, Node B's children
    // shall also be collapsed the next time we open Node A.
    // Basically propagate collapse state "down the tree".
    const core::ModelIndex root{};

    flattenedList_.clear();

    auto genFlatlist = [this](auto&& self, const core::ModelIndex r) -> void
    {
        if (r.isValid()) { flattenedList_.push_back(r); }
        if (!expandedSet_.contains(r)) { return; }

        for (uint32_t i = 0; i < model_->getRowCount(r); ++i)
        {
            const core::ModelIndex m = model_->index(i, 0, r);
            self(self, m);
        }
    };

    genFlatlist(genFlatlist, root);
}

auto UITreeView::prepareItem(const uint32_t viewRow, std::shared_ptr<UIViewItem>& item) -> void
{
    auto[expandBtn, infoBtn] = item->getButtonPair();

    core::ModelIndex idx = flattenedList_[viewRow];

    const uint32_t depth = model_->depth(idx);
    const int32_t margin = 50.0f * depth;

    const std::string expandText = model_->hasChildren(idx)
        ? expandedSet_.contains(idx) ? "^" : ">"
        : "";
    const std::string infoText = model_->data(idx) + " /\\ " + std::to_string(item->getId());
    expandBtn->setText(expandText);
    infoBtn->setText(infoText);

    // if (expandText.empty()) { expandBtn->setDisabled(); }
    // else { expandBtn->setEnabled(); }

    core::LayoutBase::ScaleXY scale
    {
        core::LayoutBase::Scale(150 + margin, core::LayoutBase::ScaleType::PX),
        // 1_fill,
        core::LayoutBase::Scale(rowSize_, core::LayoutBase::ScaleType::PX)
    };

    item->getBaseLayoutData()
        .setScale(scale)
        .setMargin({0, 0, margin, 0});
        // .setPadding({0, 0, margin, 0});
    // infoBtn->getBaseLayoutData().setScale({150_px, 1_fill});
    // expandBtn->getBaseLayoutData()
    //     .setMargin({0, 0, margin, 0});
    expandBtn->setColor(viewRow % 2
        ? utils::hexToVec4("#adadadff")
        : utils::hexToVec4("#e46b6bff"));
    infoBtn->setColor(viewRow % 2
        ? utils::hexToVec4("#adadadff")
        : utils::hexToVec4("#e46b6bff"));
    // item->setColor(viewRow % 2
    //     ? utils::hexToVec4("#adadadff")
    //     : utils::hexToVec4("#e46b6bff"));

    expandBtn->listenEvent<core::MouseLeftReleaseEvt>(
        [this, idx](const auto&)
        {
            /*
                Note: After each expansion/collapse, we need to redo the flat list and recalculate the layout
                as things changed in the UITreeView's internal structure.
            */
            expandedSet_.contains(idx) ? (void)expandedSet_.erase(idx) : (void)expandedSet_.insert(idx);
            
            oldVisibleCount_ = 0;
            computeFlatList();
            onLayout();
        });
    infoBtn->listenEvent<core::MouseLeftReleaseEvt>(
        [this, idx](const auto&)
        {
            core::ViewLMBRelease evt{idx};
            eventsMgr_.emitEvent<core::ViewLMBRelease>(evt);
        });
}

auto UITreeView::allocatePool() -> void
{
    if (uiItemPool_.size() >= static_cast<uint32_t>(visibleCount_)) { return; }

    for (int32_t i = 0; i < visibleCount_; ++i)
    {
        auto itemObj = utils::make<UIViewItem>();
        uiItemPool_.push_back(itemObj);
    }
}

auto UITreeView::setModel(const core::AbstractModelPtr model) -> void
{
    model_ = model;

    // invalid root is always in expandedSet and cannot be removed
    core::ModelIndex root{};
    expandedSet_.clear();
    expandedSet_.insert(root);

    computeFlatList();
}

auto UITreeView::setRowSize(const uint32_t value) -> void
{
    rowSize_ = value;
}
} // namespace lav::node
