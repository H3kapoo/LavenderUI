#include "UIRecycleList.hpp"

#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/LayoutHandler/Calculators/PaneCalculator.hpp"
#include "src/Node/InternalUse/UIScroll.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Utils/Misc.hpp"
#include <sstream>

namespace lav::node
{
UIRecycleList::UIRecycleList(UIBaseInitData&& initData)
    : UIPane(std::move(initData))
    , model_{nullptr}
    , selectedId_(0)
    , topOfTheListIdx_{0}
    , oldTopOfTheListIdx_{-1}
    , visibleCount_{0}
    , oldVisibleCount_{-1}
    , tolerance_{2}
    // , rowSize_{16}
    , rowSize_{28}
{
    setScrollEnabled(false, true);
    // setBorderColor(utils::hexToVec4("#eefcffff"));
    setBorderColor(utils::hexToVec4("#c0cbcdff"));
    setColor(utils::hexToVec4("#c0cbcdff"));
    // setColor(utils::hexToVec4("#eefcffff"));
    layoutBase_.setType(core::LayoutBase::Type::VERTICAL);
    layoutBase_.setBorder(4);
    // layoutBase_.setBorder({4, 4, 4, 0});

    // vScroll_->getBaseLayoutData().setBorder({0, 0, 4, 0});
    vScroll_->getBaseLayoutData().setMargin({0, 0, 4, 0});
    // vScroll_->setBorderColor(utils::hexToVec4("#df6adfff"));

    // mock
    for (int32_t i = 0; i < 160; ++i)
    {
        auto itemObj = utils::make<UIButton>();
        uiButtonPool_.push_back(itemObj);
    }
}

auto UIRecycleList::onRender(const glm::mat4& projection) -> void
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

auto UIRecycleList::onLayout() -> void
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

auto UIRecycleList::calculateLayout() -> glm::i64vec2
{
    const auto& calculator = core::PaneCalculator::get();
    glm::i64vec2 overflow{0, 0};

    const auto sliderImpact = calculator.calculateSlidersScaleAndPos(this);
    calculator.calculateScaleForGenericElement(this, sliderImpact);
    calculator.calculatePositionForGenericElement(this, sliderImpact);

    overflow = calculator.calculateElementOverflow(this, sliderImpact);

    return overflow;
}

auto UIRecycleList::onEvent(node::UIStatePtr& state) -> void
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

auto UIRecycleList::resolveVisibleItems() -> void
{
    topOfTheListIdx_ = vScroll_ ? vScroll_->getScrollValue() / rowSize_ : 0;
    // visibleCount_ = layoutBase_.getContentBoxScale().y / rowSize_ + tolerance_;
    visibleCount_ = layoutBase_.getContentBoxScale().y / rowSize_ + tolerance_;
    if (topOfTheListIdx_ == oldTopOfTheListIdx_ && visibleCount_ == oldVisibleCount_)
    {
        return;
    }

    wantedVisibleRowStartIdx_ = topOfTheListIdx_;
    wantedVisibleRowEndIdx_ = topOfTheListIdx_ + visibleCount_;

    // log_.error("CS {} CE {}", currentVisibleRowStartIdx_, currentVisibleRowEndIdx_);
    // log_.error("WS {} WE {}", wantedVisibleRowStartIdx_, wantedVisibleRowEndIdx_);

    // Decide here what to remove
    if (wantedVisibleRowStartIdx_ > currentVisibleRowStartIdx_)
    {
        // remove start
        const uint32_t diff = wantedVisibleRowStartIdx_ - currentVisibleRowStartIdx_;
        uint32_t deleted{0};
        uint32_t idx{0};
        while (diff > deleted)
        {
            if (elements_.at(idx)->getTypeId() == node::UIScroll::typeId)
            {
                idx++;
                continue;
            }
            UIBase::remove(elements_.at(idx));
            deleted++;
        }
    }

    if (wantedVisibleRowEndIdx_ < currentVisibleRowEndIdx_)
    {
        // remove end
        const uint32_t diff = currentVisibleRowEndIdx_ - wantedVisibleRowEndIdx_;
        uint32_t deleted{0};
        int32_t idx = elements_.size() - 1;
        while (diff > deleted)
        {
            if (elements_.at(idx)->getTypeId() == node::UIScroll::typeId)
            {
                idx--;
                continue;
            }

            UIBase::remove(elements_.at(idx));
            deleted++;
            idx--;
        }
    }
    
    // Decide here what to add
    if (wantedVisibleRowStartIdx_ < currentVisibleRowStartIdx_)
    {
        // add start
        uint32_t ctr{0};
        for (uint32_t i = wantedVisibleRowStartIdx_; i < currentVisibleRowStartIdx_; ++i)
        {
            UIBase::add(makeUINodeAt(i), ctr);
            ctr++;
        }
    }

    if (wantedVisibleRowEndIdx_ > currentVisibleRowEndIdx_)
    {
        // add end
        for (uint32_t i = currentVisibleRowEndIdx_; i < wantedVisibleRowEndIdx_; ++i)
        {
            UIBase::add(makeUINodeAt(i));
        }
    }

    /*
        CS 20 CE 24
        WS 17 WE 26

        18 17 20 21 22 23 24


        CS 20 CE 44
        WS 17 WE 51

        if WS < CS
            add_start(20, 19, 18)
        if WE > CE
            add_end(45, 46, 47, 48, 49, 50, 51)

        CS 17 CE 51
        WS 20 WE 44

        if WS > CS
            remove_start(17, 18, 19)
        if WE < CE
            remove_end(45, 46, 47, 48, 49 50, 51)
    */

    /*
        OldVisible:  2 3 4 5 6 7
        NewVisible:  0 1 2 3 4 5 6 7 8 9

        OldVisible:  0 1 2 3 4 5 6 7 8 9
        NewVisible:  2 3 4 5 6 7


        OldVisible:  0 1 2 3 4 5
        NewVisible:  2 3 4 5 6 7

        OldVisible:  2 3 4 5 6 7
        NewVisible:  0 1 2 3 4 5

        0 1 2 3 4 5
        if newvisible[0] < oldvisible[0]
            add_start(newVisible[0].idx -> oldVisible[0].idx non-inclusive)
        else newvisible[0] > oldvisible[0]
            remove_start(oldVisible[0] -> newVisible[0].idx non-inclusive)
        if newvisible[last] > oldvisible[last]
            add_new_end(oldvisible[last].idx non-inclusive -> newvisible[last])
        else newvisible[last] < oldvisible[last]
            rm_end(newvisible[last].idx non-inclusive -> oldvisible[last])

        don't need a vector, size doesn't care, we only care about
        oldVisibleStartRow
        oldVisibleEndRow
        
        newVisibleStartRow
        newVisibleEndRow

        this way we don't need to remove all the elements, only a particular group
    */


    oldTopOfTheListIdx_ = topOfTheListIdx_;
    oldVisibleCount_ = visibleCount_;
    currentVisibleRowStartIdx_ = wantedVisibleRowStartIdx_;
    currentVisibleRowEndIdx_ = wantedVisibleRowEndIdx_;
}

auto UIRecycleList::makeUINodeAt(const uint32_t viewRowIdx) -> UIButtonPtr
{
    if (viewRowIdx >= model_->getRowCount()) { return nullptr; }

    auto itemObj = utils::make<UIButton>();
    // auto itemObj = uiButtonPool_[viewRowIdx];

    core::ModelIndex idx = model_->index(viewRowIdx, 0, core::ModelIndex{});

    itemObj->setText(model_->data(idx));

    /* Set private stuff on the visual object. */
    using core::LayoutBase;
    LayoutBase::ScaleXY scale { 1_fill, LayoutBase::Scale(rowSize_, LayoutBase::ScaleType::PX) };

    itemObj->getBaseLayoutData().setScale(scale);
    itemObj->setColor(viewRowIdx % 2
        ? utils::hexToVec4("#adadadff")
        : utils::hexToVec4("#e46b6bff"));
    itemObj->listenEvent<core::MouseLeftReleaseEvt>(
        [this, idx](const auto&)
        {
            core::ViewLMBRelease evt{idx};
            eventsMgr_.emitEvent<core::ViewLMBRelease>(evt);
        });

    return itemObj;
}

auto UIRecycleList::getUINodeAt(uint32_t idx) -> UIBasePtr
{
    std::vector<UIBasePtr> filtered;
    std::copy_if(elements_.begin(), elements_.end(),
        std::back_inserter(filtered),
        [](const auto& obj)
        {
            return obj->getTypeId() != node::UIScroll::typeId;
        });

    if (idx >= filtered.size()) { return nullptr; }
    return filtered[idx];
}

auto UIRecycleList::setModel(const core::AbstractModelPtr model) -> void
{
    model_ = model;
}
} // namespace lav::node
