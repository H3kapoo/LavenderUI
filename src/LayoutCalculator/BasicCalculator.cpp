#include "BasicCalculator.hpp"

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
    const auto& pLayout = parent->getLayout();
    const auto& pComputedPos = pLayout.getComputedPos();
    const auto& pComputedScale = pLayout.getComputedScale() - scrollData;
    const auto& pTemp = pLayout.tempPosOffset;
    const auto& pLayoutType = pLayout.getType();
    const auto pWrap = pLayout.getWrap();
    const auto& elements = parent->getElements();

    glm::vec2 nextPos{pComputedPos + pTemp};
    glm::vec2 pos{0, 0};
    glm::vec2 maxOnAxis{0, 0};
    for (const auto& element : elements)
    {
        if (element->getTypeId() == uielements::UISlider::typeId && element->getCustomTagId() == 1000) { continue; }

        auto& eLayout = element->getLayout();
        const auto& margins = eLayout.getMargin();
        const auto& compScale = eLayout.getComputedScale();
        const glm::vec2 fullScale = eLayout.getFullBoxScale();

        if (pLayoutType == LayoutBase::Type::HORIZONTAL)
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
        else if (pLayoutType == LayoutBase::Type::VERTICAL)
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

        eLayout.setComputedPos(pos);
    }
}

auto BasicCalculator::calcElementsScale(uielements::UIBase* parent,
    const glm::vec2 scrollData) const -> void
{
    const auto& pLayout = parent->getLayout();
    const auto& pComputedScale = pLayout.getComputedScale() - scrollData;
    // const auto& pComputedPos = pLayout.getComputedPos();
    // const auto& pLayoutType = pLayout.getType();
    const auto& elements = parent->getElements();

    for (const auto& element : elements)
    {
        if (element->getTypeId() == uielements::UISlider::typeId && element->getCustomTagId() == 1000) { continue; }

        glm::vec2 cScale;
        auto& eLayout = element->getLayout();
        const auto& userScale = eLayout.getScale();
        // const auto& marginTB = eLayout.getTBMargin();
        const auto& marginLR = eLayout.getLRMargin();
        if (userScale.x.type == LayoutBase::ScaleType::PX)
        {
            cScale.x = userScale.x.val;// - marginLR;
        }
        else if (userScale.x.type == LayoutBase::ScaleType::REL)
        {
            cScale.x = pComputedScale.x * userScale.x.val - marginLR;
        }
        else if (userScale.x.type == LayoutBase::ScaleType::FIT)
        {

        }
        else if (userScale.x.type == LayoutBase::ScaleType::FILL)
        {

        }

        // y
        if (userScale.y.type == LayoutBase::ScaleType::PX)
        {
            cScale.y = userScale.y.val;// - marginTB;
        }
        else if (userScale.y.type == LayoutBase::ScaleType::REL)
        {
            cScale.y = pComputedScale.y * userScale.y.val;
        }
        else if (userScale.y.type == LayoutBase::ScaleType::FIT)
        {

        }
        else if (userScale.y.type == LayoutBase::ScaleType::FILL)
        {

        }

        eLayout.setComputedScale(cScale);
    }
}

auto BasicCalculator::calcOverflow(uielements::UIBase* parent,
    const glm::vec2 scrollData) const -> glm::vec2
{
    glm::vec2 boxScale{0, 0};
    const auto& pLayout = parent->getLayout();
    const auto& pContentPos = pLayout.getContentBoxPos();
    const auto& pContentScale = pLayout.getContentBoxScale() - scrollData;
    const auto& elements = parent->getElements();
    for (const auto& element : elements)
    {
        /* Shall not be taken into consideration for overflow */
        if (element->getTypeId() == uielements::UISlider::typeId && element->getCustomTagId() == 1000) { continue; }

        const auto& eLayout = element->getLayout();
        const auto& fullPos = eLayout.getFullBoxPos();
        const auto& fullScale = eLayout.getFullBoxScale();
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
    const auto& pLayout = parent->getLayout();
    const auto& pComputedPos = pLayout.getComputedPos();
    const auto& pComputedScale = pLayout.getComputedScale();
    if (const auto vSlider = parent->getVerticalSlider().lock(); vSlider && vSlider->isParented())
    {
        // Scroll sliders on a Pane can ONLY have PX values on the scroll direction.
        auto& eLayout = vSlider->getLayout();
        sliderImpact.x = eLayout.getScale().x.val;
        eLayout.setComputedPos({
            pComputedPos.x + pComputedScale.x - sliderImpact.x,
            pComputedPos.y
        });
        eLayout.setComputedScale({sliderImpact.x, pComputedScale.y});
    }

    if (const auto hSlider = parent->getHorizontalSlider().lock(); hSlider && hSlider->isParented())
    {
        // Scroll sliders on a Pane can ONLY have PX values on the scroll direction.
        auto& eLayout = hSlider->getLayout();
        sliderImpact.y = eLayout.getScale().y.val;
        eLayout.setComputedPos({
            pComputedPos.x,
            pComputedPos.y + pComputedScale.y - sliderImpact.y,
        });
        eLayout.setComputedScale({pComputedScale.x - sliderImpact.x, sliderImpact.y});
    }

    return sliderImpact;
}

auto BasicCalculator::calcPaneElementsAddScrollToPos(uielements::UIPane* parent,
    const glm::vec2 scrollData) const -> void
{
    const auto& elements = parent->getElements();
    for (const auto& element : elements)
    {
        if (element->getTypeId() == uielements::UISlider::typeId && element->getCustomTagId() == 1000) { continue; }\

        auto& eLayout = element->getLayout();
        eLayout.setComputedPos(eLayout.getComputedPos() - scrollData);
    }
}
} // namespace src::layoutcalculator
