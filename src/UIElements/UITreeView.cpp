#include "UITreeView.hpp"

#include "src/ElementComposable/IEvent.hpp"
#include "src/LayoutCalculator/BasicCalculator.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UISlider.hpp"
#include "src/WindowManagement/Input.hpp"

namespace src::uielements
{
UITreeView::UITreeView()
    : UIPane(getTypeInfo())
{
    setType(LayoutBase::Type::VERTICAL);
}

auto UITreeView::render(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", getTransform());
    shader_.uploadVec4f("uColor", getColor());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    static auto l = [](const UIBasePtr& e) { return e->getCustomTagId () == UISlider::scrollTagId; };
    renderNextExcept(projection, l);
    renderNextSingle(projection, vSlider_);
    renderNextSingle(projection, hSlider_);
}

auto UITreeView::layout() -> void
{
    using namespace layoutcalculator;
    const auto& calculator = BasicCalculator::get();

    resolveVisibleItems();

    glm::ivec2 overflow{0, 0};
    do
    {
        const auto sliderImpact = calculator.calculateSlidersScaleAndPos(this);
        calculator.calculateScaleForGenericElement(this, sliderImpact);
        calculator.calculatePositionForGenericElement(this, sliderImpact);

        overflow = calculator.calculateElementOverflow(this, sliderImpact);
        overflow.y = flatItems_.size() * rowSize_ - getComputedScale().y;
    
    } while (updateSlidersWithOverflow(overflow));

    calculator.calculateElementsOffsetDueToScroll(this, {
        hSlider_ ? hSlider_->getScrollValue() : 0,
        vSlider_ ? (uint32_t)vSlider_->getScrollValue() % rowSize_ : 0});

    layoutNext();
}

auto UITreeView::event(state::UIWindowStatePtr& state) -> void
{
    /* Let the base do the generic stuff like mouse move pre-pass. */
    UIBase::event(state);

    updateClosestSlider(state);

    // const auto eId = state->currentEventId;

    eventNext(state);
}


auto UITreeView::resolveVisibleItems() -> void
{
    /* Slider value needs to be reset to zero if there's no need for it anymore after an
    item has closed. */
    if (flatItems_.size() * rowSize_ - getComputedScale().y <= 0)
    {
        vSlider_ ? vSlider_->setScrollValue(0) : void();
        // TODO: Not ok if there's still overflow on X axis
        hSlider_ ? hSlider_->setScrollValue(0) : void();
    }

    topOfTheListIdx_ = vSlider_ ? vSlider_->getScrollValue() / rowSize_ : 0;
    visibleCount_ = getComputedScale().y / rowSize_ + 2;

    if (topOfTheListIdx_ == oldTopOfTheListIdx_ && visibleCount_ == oldVisibleCount_)
    {
        return;
    }

    remove([](const auto&){ return true; });

    for (int32_t i = 0; i < visibleCount_; ++i)
    {
        uint32_t index = topOfTheListIdx_ + i;
        if (index >= flatItems_.size()) { break; }

        // auto ref = std::make_shared<Button>("Item");
        auto itemObj = utils::make<UIButton>();
        // auto itemObj = utils::make<UISlider>();
        itemObj->setColor(flatItems_[index]->color);
        itemObj->setText(flatItems_[index]->text);
        itemObj->setScale({.x = 200_px, .y = {(float)rowSize_}})
            .setMargin({0, 0, flatItems_[index]->depth * 20, 0});

        itemObj->listenTo<elementcomposable::MouseButtonEvt>(
            [this, index](const auto& e)
            {
                using namespace windowmanagement;
                if (e.action == Input::RELEASE)
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

auto UITreeView::addItem(ItemPtr&& item) -> void
{
    treeRoots_.emplace_back(item);
}

auto UITreeView::addItem(const ItemPtr& item) -> void
{
    treeRoots_.emplace_back(item);
}

auto UITreeView::removeItem(const ItemPtr& item) -> bool
{
    return std::erase(treeRoots_, item);
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

    /* Note: An optimization could be done in the future such that tree[index] gives us
        the visible element that would be at that index if the tree was flat.
        A custom class iterator maybe for Item? This will half the memory needed to store items. */

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
}

auto UITreeView::Item::addItem(ItemPtr&& item) -> void
{
    subItems.emplace_back(std::move(item));
}

auto UITreeView::Item::addItem(const ItemPtr& item) -> void
{
    subItems.emplace_back(item);
}

auto UITreeView::Item::removeItem(const ItemPtr& item) -> bool
{
    return std::erase(subItems, item);
}
} // namespace src::uielements
