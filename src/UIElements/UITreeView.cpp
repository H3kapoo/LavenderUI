#include "UITreeView.hpp"

#include "src/ElementComposable/IEvent.hpp"
#include "src/LayoutCalculator/BasicCalculator.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "src/UIElements/UISlider.hpp"
#include "src/Utils/Misc.hpp"
#include "vendor/glfw/include/GLFW/glfw3.h"

namespace src::uielements
{
UITreeView::UITreeView()
    : UIPane(getTypeInfo())
{
    layoutBase_.setType(LayoutBase::Type::VERTICAL);
}

auto UITreeView::render(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", propsBase_.getColor());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    static auto l = [](const UIBasePtr& e) { return e->getCustomTagId () == 1000; };
    renderNextExcept(projection, l);
    renderNextSingle(projection, vSlider_);
    renderNextSingle(projection, hSlider_);
}

auto UITreeView::layout() -> void
{
    using namespace layoutcalculator;
    const auto& calculator = BasicCalculator::get();

    /* Slider value needs to be reset to zero if there's no need for it anymore after an
    item had closed */
    if (flatItems_.size() * rowSize_ - layoutBase_.getComputedScale().y <= 0)
    {
        vSlider_ ? vSlider_->setScrollValue(0) : void();
        hSlider_ ? hSlider_->setScrollValue(0) : void();
    }

    topOfTheListIdx_ = vSlider_ ? vSlider_->getScrollValue() / rowSize_ : 0;
    visibleCount_ = layoutBase_.getComputedScale().y / rowSize_ + 2;
    if (topOfTheListIdx_ != oldTopOfTheListIdx_ || visibleCount_ != oldVisibleCount_)
    {
        log_.debug("tol {} {}", topOfTheListIdx_, visibleCount_);
        remove([](const auto&){ return true; });

        for (int32_t i = 0; i < visibleCount_; ++i)
        {
            uint32_t index = topOfTheListIdx_ + i;
            if (index >= flatItems_.size()) { break; }

            // auto ref = std::make_shared<Button>("Item");
            // auto itemObj = utils::make<UIButton>();
            auto itemObj = utils::make<UISlider>();
            itemObj->getProps().setColor(flatItems_[index]->color);
            itemObj->setText(flatItems_[index]->text);
            itemObj->getLayout()
                .setScale({.x = 200_px, .y = {(float)rowSize_}})
                .setMargin({0, 0, flatItems_[index]->depth * 20, 0});

            itemObj->getEvents().listenTo<elementcomposable::MouseButtonEvt>(
                [this, index](const auto& e)
                {
                    if (e.action == GLFW_RELEASE)
                    {
                        log_.debug("clicked on {}", flatItems_[index]->text);
                        flatItems_[index]->open = !flatItems_[index]->open;
                        oldVisibleCount_ = 0;
                        refreshItems();

                        layout();
                    }
                });
            add(itemObj);
        }
        oldTopOfTheListIdx_ = topOfTheListIdx_;
        oldVisibleCount_ = visibleCount_;
    }

    /* Adding new elements (slides in this case) invalidates the calculations. */
    while (true)
    {
        const auto sliderImpact = calculator.calcPaneSliders(this);
        calculator.calcPaneElements(this, sliderImpact);

        glm::vec2 overflow = calculator.calcOverflow(this, sliderImpact);
        overflow.y = flatItems_.size() * rowSize_ - layoutBase_.getComputedScale().y;
        if (const auto needsRecalc = updateSlidersWithOverflow(overflow); !needsRecalc) { break; }
    }
    
    calculator.calcPaneElementsAddScrollToPos(this, {
        hSlider_ ? hSlider_->getScrollValue() : 0,
        vSlider_ ? (uint32_t)vSlider_->getScrollValue() % rowSize_ : 0});

    layoutNext();
}

auto UITreeView::event(framestate::FrameStatePtr& state) -> void
{
    /* Let the base do the generic stuff like mouse move pre-pass. */
    UIBase::event(state);

    updateClosestSlider(state);

    // const auto eId = state->currentEventId;

    eventNext(state);
}

auto UITreeView::addItem(const ItemPtr& item) -> void
{
    treeRoots_.emplace_back(item);
}

auto UITreeView::refreshItems() -> void
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

    flatItems_.clear();
    auto recurseFlat = [this](const auto& self, const ItemPtrVec& items, const int32_t depth) -> void
    {
        for (const auto& item : items)
        {
            item->depth = depth;
            flatItems_.emplace_back(item);
            if (item->open)
            {
                self(self, item->subItems, depth + 1);
            }
        }
    };
    recurseFlat(recurseFlat, treeRoots_, 0);

    log_.debug("size {}", flatItems_.size());
    // std::ranges::for_each(flatItems_, [this](const auto& e)
    // {
    //     log_.debug("Item: {}", e->text);
    // });
}

} // namespace src::uielements
