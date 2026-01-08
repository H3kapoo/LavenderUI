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

    core::LayoutBase::ScaleXY scale
    {
        1_fill,
        core::LayoutBase::Scale(rowSize_, core::LayoutBase::ScaleType::PX)
    };


    for (int32_t i = 0; i < visibleCount_; ++i)
    {
        uint64_t viewRow = topOfTheListIdx_ + i;
        if (viewRow >= flattenedList_.size()) { break; }

        auto itemObj = utils::make<UIButton>();

        // core::ModelIndex idx = model_->index(viewRow, 0, core::ModelIndex{});
        core::ModelIndex idx = flattenedList_[viewRow];

        itemObj->setText(model_->data(idx));

        /* Set private stuff on the visual object. */
        itemObj->getBaseLayoutData().setScale(scale);
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

    /*
        - Root
            - Root_A
                - A_0
                - A_1
            - Root_B
            - Root_C
                - C_0
                - C_1
                - C_2
                    - C2_Child_0
                    - C2_Child_1
                    - C2_Child_2
                - C_3
                - C_4
                - C_5

        [ Root, Root_A, A_0, A_1, Root_B, Root_C, C_0, C_1, C_2, C2_Child_0, C2_Child_1, C2_Child_2,
            C_3, C_4, C_5 ]
    */

    auto recurse = [this](auto&& self, const core::ModelIndex r) -> void
    {
        // log_.error("pushing {} {}", r.row, r.internalPtr ? "not_root" : "root");
        log_.error("rows in r {}", model_->data(r));
        flattenedList_.push_back(r);
        // const uint32_t rows = model_->getRowCount(r);
        for (uint32_t i = 0; i < model_->getRowCount(r); ++i)
        {
            const core::ModelIndex m = model_->index(i, 0, r);
            //TODO: Something is wrong if we get root back again by this time
            // log_.error("we have {} rows in {} ", model_->getRowCount(m), model_->data(m));

            self(self, m);
        }
    };

    recurse(recurse, root);

    /*

        MI(MAX, MAX, nullptr) -> 3 rows Root
            MI(0, 0, root_) -> 2 rows Root_A
                MI(0, 0, Root_A) -> A_0



        ModelIndex{MAX, MAX, nullptr} -> 3 rows (parent) -> Root
            ModelIndex(0, 0, root) -> 2 rows, go back
                ModelIndex(0, 0, ModelIndex(0, 0, root)) -> 2 rows, go back
            ModelIndex(1, 0, root) -> 0 rows, go back
            ModelIndex(2, 0, root) -> 0 rows, go back
            
    */

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
