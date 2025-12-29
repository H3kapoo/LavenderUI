#include "UIRecycleList.hpp"

#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/LayoutHandler/Calculators/PaneCalculator.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::node
{
UIRecycleList::UIRecycleList(UIBaseInitData&& initData)
    : UIPane(std::move(initData))
{
    layoutBase_.setType(core::LayoutBase::Type::VERTICAL);
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
    if (model_->getItemsCount() * rowSize_ - layoutBase_.getComputedScale().y <= 0)
    {
        vScroll_ ? vScroll_->setScrollValue(0) : void();
    }

    glm::i64vec2 overflow{0, 0};
    overflow.y = model_->getItemsCount() * rowSize_ - layoutBase_.getComputedScale().y;
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
    using namespace lav::core;

    topOfTheListIdx_ = vScroll_ ? vScroll_->getScrollValue() / rowSize_ : 0;
    visibleCount_ = layoutBase_.getComputedScale().y / rowSize_ + tolerance_;
    if (topOfTheListIdx_ == oldTopOfTheListIdx_ && visibleCount_ == oldVisibleCount_)
    {
        return;
    }

    UIBase::remove([this](const auto& e)
    {
        return e->getId() != vScroll_->getId() && e->getId() != hScroll_->getId();
    });

    LayoutBase::ScaleXY scale
    {
        1_fill,
        LayoutBase::Scale(rowSize_, LayoutBase::ScaleType::PX)
    };
    for (int32_t i = 0; i < visibleCount_; ++i)
    {
        uint64_t index = topOfTheListIdx_ + i;
        if (index >= model_->getItemsCount()) { break; }

        auto itemObj = utils::make<UIButton>();

        model_->dataForIndex(itemObj, index);

        itemObj->getBaseLayoutData()
            .setScale(scale);

        UIBase::add(itemObj);
    }
    oldTopOfTheListIdx_ = topOfTheListIdx_;
    oldVisibleCount_ = visibleCount_;
}

auto UIRecycleList::setModel(std::unique_ptr<AbstractModel> model) -> void
{
    model_ = std::move(model);
}
} // namespace lav::node
