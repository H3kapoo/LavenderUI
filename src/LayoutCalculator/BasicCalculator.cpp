#include "BasicCalculator.hpp"
#include "src/ElementComposable/LayoutAttribs.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UISlider.hpp"
#include "src/Utils/Logger.hpp"

namespace src::layoutcalculator
{
using namespace elementcomposable;

auto BasicCalculator::get() -> BasicCalculator&
{
    static BasicCalculator instance;
    return instance;
}

auto BasicCalculator::calculate(const uielements::UIBasePtr& parent) -> void
{
    glm::vec2 scrollData{0,0};// = calcPaneSliders(parent.get());
    calcElementsScale(parent.get(), scrollData);
    calcElementsPos(parent.get(), scrollData);
}

auto BasicCalculator::calcElementsPos(uielements::UIBase* parent,
    const glm::vec2 scrollData) const -> void
{
    const auto& pComputedPos = parent->getLayoutComputedPos();
    const auto& pComputedScale = parent->getLayoutComputedScale() - scrollData;
    const auto& pTemp = parent->tempPosOffset;
    const auto& pLayoutType = parent->getLayoutType();
    const auto pWrap = parent->getLayoutWrap();
    const auto& elements = parent->getElements();

    glm::vec2 nextPos{pComputedPos + pTemp};
    glm::vec2 pos{0, 0};
    glm::vec2 maxOnAxis{0, 0};
    for (const auto& element : elements)
    {
        if (element->getTypeId() == uielements::UISlider::typeId) { continue; }

        const auto& margins = element->getLayoutMargin();
        const auto& compScale = element->getLayoutComputedScale();
        const glm::vec2 fullScale = element->getFullBoxScale();

        if (pLayoutType == LayoutAttribs::Type::HORIZONTAL)
        {
            // nextPos starts at the end of the previous' element margin end
            if (pWrap && nextPos.x + fullScale.x > pComputedPos.x + pComputedScale.x)
            {
                nextPos.y += maxOnAxis.y;
                nextPos.x = pComputedPos.x + pTemp.x;
                maxOnAxis.y = 0;
            }

            pos = nextPos + glm::vec2{margins.left, margins.top};
            nextPos.x = pos.x + compScale.x + margins.right;
            maxOnAxis.y = std::max(maxOnAxis.y, fullScale.y);
        }
        else if (pLayoutType == LayoutAttribs::Type::VERTICAL)
        {
            // nextPos starts at the end of the previous' element margin end
            if (pWrap && nextPos.y + fullScale.y > pComputedPos.y + pComputedScale.y)
            {
                nextPos.x += maxOnAxis.x;
                nextPos.y = pComputedPos.y + pTemp.y;
                maxOnAxis.x = 0;
            }

            pos = nextPos + glm::vec2{margins.left, margins.top};
            nextPos.y = pos.y + compScale.y + margins.bot;
            maxOnAxis.x = std::max(maxOnAxis.x, fullScale.x);
        }

        element->setLayoutComputedPos(pos);
    }
}

auto BasicCalculator::calcElementsScale(uielements::UIBase* parent,
    const glm::vec2 scrollData) const -> void
{
    const auto& pComputedScale = parent->getLayoutComputedScale() - scrollData;
    const auto& pComputedPos = parent-> getLayoutComputedPos();
    const auto& pLayoutType = parent->getLayoutType();
    const auto& elements = parent->getElements();

    for (const auto& element : elements)
    {
        if (element->getTypeId() == uielements::UISlider::typeId) { continue; }

        const auto& userScale = element->getLayoutScale();
        const auto& marginTB = element->getTBMargin();
        const auto& marginLR = element->getLRMargin();
        glm::vec2 cScale;
        if (userScale.x.type == LayoutAttribs::ScaleType::PX)
        {
            cScale.x = userScale.x.val;// - marginLR;
        }
        else if (userScale.x.type == LayoutAttribs::ScaleType::REL)
        {
            cScale.x = pComputedScale.x * userScale.x.val;
        }
        else if (userScale.x.type == LayoutAttribs::ScaleType::FIT)
        {

        }
        else if (userScale.x.type == LayoutAttribs::ScaleType::FILL)
        {

        }

        // y
        if (userScale.y.type == LayoutAttribs::ScaleType::PX)
        {
            cScale.y = userScale.y.val;// - marginTB;
        }
        else if (userScale.y.type == LayoutAttribs::ScaleType::REL)
        {
            cScale.y = pComputedScale.y * userScale.y.val;
        }
        else if (userScale.y.type == LayoutAttribs::ScaleType::FIT)
        {

        }
        else if (userScale.y.type == LayoutAttribs::ScaleType::FILL)
        {

        }

        element->setLayoutComputedScale(cScale);
    }
}

auto BasicCalculator::calcOverflow(uielements::UIBase* parent,
    const glm::vec2 scrollData) const -> glm::vec2
{
    glm::vec2 boxScale{0, 0};
    const auto& pContentPos = parent->getContentBoxPos();
    const auto& pContentScale = parent->getContentBoxScale() - scrollData;
    const auto& elements = parent->getElements();
    for (const auto& element : elements)
    {
        /* Shall not be taken into consideration for overflow */
        if (element->getTypeId() == uielements::UISlider::typeId) { continue; }

        const auto& fullPos = element->getFullBoxPos();
        const auto& fullScale = element->getFullBoxScale();
        boxScale.x = std::max(boxScale.x, fullPos.x + fullScale.x);
        boxScale.y = std::max(boxScale.y, fullPos.y + fullScale.y);
    }

    return boxScale - (pContentPos + pContentScale);
}

auto BasicCalculator::calcPaneElements(uielements::UIPane* parent,
    const glm::vec2 scrollData) const -> void
{
    calcElementsScale(parent, scrollData);
    calcElementsPos(parent, scrollData);
}

auto BasicCalculator::calcPaneSliders(uielements::UIPane* parent) const -> glm::vec2
{
    glm::vec2 sliderImpact{0, 0};
    const auto& pComputedPos = parent->getLayoutComputedPos();
    const auto& pComputedScale = parent->getLayoutComputedScale();
    if (const auto vSlider = parent->getVerticalSlider().lock(); vSlider->isParented())
    {
        // Scroll sliders on a Pane can ONLY have PX values on the scroll direction.
        sliderImpact.x = vSlider->getLayoutScale().x.val;
        vSlider->setLayoutComputedPos({
            pComputedPos.x + pComputedScale.x - sliderImpact.x,
            pComputedPos.y
        });
        vSlider->setLayoutComputedScale({sliderImpact.x, pComputedScale.y});
    }

    if (const auto hSlider = parent->getHorizontalSlider().lock(); hSlider->isParented())
    {
        // Scroll sliders on a Pane can ONLY have PX values on the scroll direction.
        sliderImpact.y = hSlider->getLayoutScale().y.val;
        hSlider->setLayoutComputedPos({
            pComputedPos.x,
            pComputedPos.y + pComputedScale.y - sliderImpact.y,
        });
        hSlider->setLayoutComputedScale({pComputedScale.x - sliderImpact.x, sliderImpact.y});
    }

    return sliderImpact;
}

auto BasicCalculator::calcPaneElementsAddScrollToPos(uielements::UIPane* parent,
    const glm::vec2 scrollData) const -> void
{
    const auto& elements = parent->getElements();
    for (const auto& element : elements)
    {
        if (element->getTypeId() == uielements::UISlider::typeId) { continue; }
        element->setLayoutComputedPos(element->getLayoutComputedPos() - scrollData);
    }
}
} // namespace src::layoutcalculator
