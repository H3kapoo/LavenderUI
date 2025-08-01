#include "BasicCalculator.hpp"
#include "src/ElementComposable/LayoutBase.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UISlider.hpp"
#include "src/Utils/Logger.hpp"

namespace src::layoutcalculator
{
using namespace elementcomposable;

#define SKIP_SLIDER(element)\
    if (element->getTypeId() == uielements::UISlider::typeId\
    && element->getCustomTagId() == uielements::UISlider::scrollTagId)\
    { continue; }\

auto BasicCalculator::get() -> BasicCalculator&
{
    static BasicCalculator instance;
    return instance;
}

auto BasicCalculator::calculateScaleForGenericElement(uielements::UIPane* parent,
    const glm::vec2 shrinkScaleBy) const -> glm::vec2
{
    const auto& pContentBoxScale = parent->getContentBoxScale() - shrinkScaleBy;
    const auto& pContentBoxPos = parent-> getContentBoxPos();
    const auto& pLayoutType = parent->getType();
    const auto& elements = parent->getElements();

    glm::vec2 runningMax{0, 0};
    for (const auto& element : elements)
    {
        SKIP_SLIDER(element);

        const auto& userScale = element->getScale();
        const auto& marginTB = element->getTBMargin();
        const auto& marginLR = element->getLRMargin();
        glm::vec2 cScale;
        if (userScale.x.type == LayoutBase::ScaleType::PX)
        {
            cScale.x = userScale.x.val - marginLR;
        }
        else if (userScale.x.type == LayoutBase::ScaleType::REL)
        {
            cScale.x = pContentBoxScale.x * userScale.x.val - marginLR;
        }
        else if (userScale.x.type == LayoutBase::ScaleType::FIT)
        {
            //TODO:
        }
        else if (userScale.x.type == LayoutBase::ScaleType::FILL)
        {
            //TODO:
        }

        // y
        if (userScale.y.type == LayoutBase::ScaleType::PX)
        {
            cScale.y = userScale.y.val - marginTB;
        }
        else if (userScale.y.type == LayoutBase::ScaleType::REL)
        {
            cScale.y = pContentBoxScale.y * userScale.y.val - marginTB;
        }
        else if (userScale.y.type == LayoutBase::ScaleType::FIT)
        {

        }
        else if (userScale.y.type == LayoutBase::ScaleType::FILL)
        {

        }

        element->setComputedScale(cScale);

        runningMax += cScale;
    }

    return runningMax;
}

auto BasicCalculator::calculatePositionForGenericElement(uielements::UIPane* parent,
    const glm::vec2 maxScale, const glm::vec2 shrinkScaleBy) const -> void
{
    const auto& pContentBoxPos = parent->getContentBoxPos();
    const auto& pContentBoxScale = parent->getContentBoxScale() - shrinkScaleBy;
    const auto& pContentBoxMaxPoint = pContentBoxPos + pContentBoxScale;
    const auto& pLayoutType = parent->getType();
    const auto pWrap = parent->getWrap();
    const auto& elements = parent->getElements();

    glm::vec2 nextPos{pContentBoxPos};
    glm::vec2 computedPos{0, 0};
    glm::vec2 maxOnAxis{0, 0};
    glm::vec2 spacing{0, 0};
    if (pLayoutType == LayoutBase::Type::HORIZONTAL)
    {
        spacing = computeSpacingOnAxis(parent->getSpacing(), elements.size(), parent->getContentBoxScale().x, maxScale.x);
        nextPos.x += spacing.y /* Additional start push*/;
    }
    else if (pLayoutType == LayoutBase::Type::VERTICAL)
    {
        spacing = computeSpacingOnAxis(parent->getSpacing(), elements.size(), parent->getContentBoxScale().y, maxScale.y);
        nextPos.y += spacing.y /* Additional start push*/;
    }

    for (auto& element : elements)
    {
        SKIP_SLIDER(element);

        const auto& margins = element->getMargin();
        const auto& compScale = element->getComputedScale();
        const glm::vec2 fullBoxScale = element->getFullBoxScale();
        const glm::vec2 nextMaxPoint = nextPos + fullBoxScale;
        const glm::vec2 marginPush = glm::vec2{margins.left, margins.top};

        /* Note: `nextPos` starts at the end of the previous' element margin end. */
        if (pLayoutType == LayoutBase::Type::HORIZONTAL)
        {
            if (pWrap && nextMaxPoint.x > pContentBoxMaxPoint.x)
            {
                nextPos.y += maxOnAxis.y;
                nextPos.x = pContentBoxPos.x;
                maxOnAxis.y = 0;
            }

            computedPos = nextPos + marginPush;
            nextPos.x = computedPos.x + compScale.x + margins.right + spacing.x;
            maxOnAxis.y = std::max(maxOnAxis.y, fullBoxScale.y);
        }
        else if (pLayoutType == LayoutBase::Type::VERTICAL)
        {
            if (pWrap && nextMaxPoint.y > pContentBoxMaxPoint.y)
            {
                nextPos.x += maxOnAxis.x;
                nextPos.y = pContentBoxPos.y;
                maxOnAxis.x = 0;
            }

            computedPos = nextPos + marginPush;
            nextPos.y = computedPos.y + compScale.y + margins.bot;
            maxOnAxis.x = std::max(maxOnAxis.x, fullBoxScale.x);
        }

        element->setComputedPos(computedPos);
    }
}

auto BasicCalculator::calculateAlignmentForElements(uielements::UIBase* parent,
    const glm::vec2 overflow) const -> void
{
    if (overflow.x >= 0 && overflow.y >= 0) { return;}

    /* Note: negative overflow means there's `-overflow` pixels left until an overflow occurs.
        We can leverage that to align elements.*/
    const auto& elements = parent->getElements();
    const auto& pAlign = parent->getAlign();
    const auto& pType = parent->getType();
    const auto& isTightSpacing = parent->getSpacing() == LayoutBase::Spacing::TIGHT;
    for (auto& element : elements)
    {
        SKIP_SLIDER(element);

        glm::vec2 offset{0, 0};
        switch (pAlign)
        {
            case LayoutBase::TOP_LEFT:
                break;
            case LayoutBase::CENTER_LEFT:
                offset.y = overflow.y < 0 ? -overflow.y * 0.5f : 0.0f;
                break;
            case LayoutBase::BOTTOM_LEFT:
                offset.y = overflow.y < 0 ? -overflow.y : 0.0f;
                break;
            case LayoutBase::TOP_CENTER:
                offset.x = overflow.x < 0 ? -overflow.x * 0.5f : 0.0f;
                break;
            case LayoutBase::CENTER:
                offset.x = overflow.x < 0 ? -overflow.x * 0.5f : 0.0f;
                offset.y = overflow.y < 0 ? -overflow.y * 0.5f : 0.0f;
                break;
            case LayoutBase::BOTTOM_CENTER:
                offset.x = overflow.x < 0 ? -overflow.x * 0.5f : 0.0f;
                offset.y = overflow.y < 0 ? -overflow.y : 0.0f;
                break;
            case LayoutBase::TOP_RIGHT:
                offset.x = overflow.x < 0 ? -overflow.x : 0.0f;
                break;
            case LayoutBase::CENTER_RIGHT:
                offset.x = overflow.x < 0 ? -overflow.x : 0.0f;
                offset.y = overflow.y < 0 ? -overflow.y * 0.5f : 0.0f;
                break;
            case LayoutBase::BOTTOM_RIGHT:
                offset.x = overflow.x < 0 ? -overflow.x : 0.0f;
                offset.y = overflow.y < 0 ? -overflow.y : 0.0f;
                break;
        }

        if (pType == LayoutBase::Type::HORIZONTAL && !isTightSpacing)
        {
            offset.x = 0;
        }
        else if (pType == LayoutBase::Type::VERTICAL && !isTightSpacing)
        {
            offset.y = 0;
        }

        element->setComputedPos(element->getComputedPos() + offset);
    }
    // utils::Logger("calc").warn("of is {}", overflow);
}

auto BasicCalculator::computeSpacingOnAxis(const LayoutBase::Spacing spacing, const int32_t elementsCount,
    const float pContentScale, const float maxScale) const -> glm::vec2
{
    float computedSpacing{0};
    float additionalStartPush{0};
    switch (spacing)
    {
        case LayoutBase::Spacing::TIGHT:
            // Do nothing
            break;
        case LayoutBase::Spacing::EVEN_NO_GAP:
            computedSpacing = (pContentScale - maxScale) / (elementsCount - 1);
            computedSpacing = std::max(0.0f, computedSpacing);
            break;
        case LayoutBase::Spacing::EVEN_GAP:
            computedSpacing = (pContentScale - maxScale) / elementsCount;
            computedSpacing = std::max(0.0f, computedSpacing);
            additionalStartPush += computedSpacing * 0.5f;
            break;
    }

    return {computedSpacing, additionalStartPush};
}

auto BasicCalculator::calcElementsPos(uielements::UIBase* parent,
    const glm::vec2 scrollData) const -> void
{
    const auto& pComputedPos = parent->getComputedPos();
    const auto& pComputedScale = parent->getComputedScale() - scrollData;
    const auto& pTemp = parent->tempPosOffset;
    const auto& pLayoutType = parent->getType();
    const auto pWrap = parent->getWrap();
    const auto& elements = parent->getElements();

    glm::vec2 nextPos{pComputedPos + pTemp};
    glm::vec2 pos{0, 0};
    glm::vec2 maxOnAxis{0, 0};
    for (auto& element : elements)
    {
        if (element->getTypeId() == uielements::UISlider::typeId
            && element->getCustomTagId() == uielements::UISlider::scrollTagId)
        { continue; }

        const auto& margins = element->getMargin();
        const auto& compScale = element->getComputedScale();
        const glm::vec2 fullScale = element->getFullBoxScale();

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

        element->setComputedPos(pos);
    }
}

auto BasicCalculator::calcElementsScale(uielements::UIBase* parent,
    const glm::vec2 scrollData) const -> void
{
    const auto& pComputedScale = parent->getComputedScale() - scrollData;
    const auto& pComputedPos = parent-> getComputedPos();
    const auto& pLayoutType = parent->getType();
    const auto& elements = parent->getElements();

    for (const auto& element : elements)
    {
        if (element->getTypeId() == uielements::UISlider::typeId
            && element->getCustomTagId() == uielements::UISlider::scrollTagId)
        { continue; }

        const auto& userScale = element->getScale();
        const auto& marginTB = element->getTBMargin();
        const auto& marginLR = element->getLRMargin();
        glm::vec2 cScale;
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

        element->setComputedScale(cScale);
    }
}

auto BasicCalculator::calcSplitPaneElements(uielements::UISplitPane* parent) const -> void
{
    const auto& pLayoutType = parent->getType();
    const auto& pComputedScale = parent->getComputedScale();
    const auto& elements = parent->getElements();

    // scale
    glm::vec2 handlesSize{0, 0};
    for (const auto& element : elements)
    {
        if (element->getTypeId() != uielements::UIButton::typeId) { continue; }

        glm::vec2 cScale;
        const auto& userScale = element->getScale();

        if (pLayoutType == LayoutBase::Type::HORIZONTAL)
        {
            cScale.x = userScale.x.val;
            cScale.y = pComputedScale.y * userScale.y.val;
        }

        handlesSize += cScale;
        element->setComputedScale(cScale);
    }

    const auto& reducedPComputedScale = pComputedScale - handlesSize;
    for (const auto& element : elements)
    {
        if (element->getTypeId() != uielements::UIPane::typeId) { continue; }

        const auto& userScale = element->getScale();
        glm::vec2 cScale;

        if (pLayoutType == LayoutBase::Type::HORIZONTAL)
        {
            cScale.x = reducedPComputedScale.x * userScale.x.val;
            cScale.y = pComputedScale.y * userScale.y.val;
        }

        element->setComputedScale(cScale);
    }

    // pos
    const auto& pComputedPos = parent->getComputedPos();

    glm::vec2 nextPos{pComputedPos};
    glm::vec2 pos{0, 0};
    for (auto& element : elements)
    {
        const auto& margins = element->getMargin();
        const auto& compScale = element->getComputedScale();
        const glm::vec2 fullScale = element->getFullBoxScale();

        if (pLayoutType == LayoutBase::Type::HORIZONTAL)
        {
            // nextPos starts at the end of the previous' element margin end
            pos = nextPos + glm::vec2{margins.left, margins.top};
            nextPos.x = pos.x + compScale.x + margins.right;
        }
        else if (pLayoutType == LayoutBase::Type::VERTICAL)
        {
            pos = nextPos + glm::vec2{margins.left, margins.top};
            nextPos.y = pos.y + compScale.y + margins.bot;
        }

        element->setComputedPos(pos);
    }
}

auto BasicCalculator::calcOverflow(uielements::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> glm::vec2
{
    glm::vec2 boxScale{0, 0};
    const auto& pContentPos = parent->getContentBoxPos();
    const auto& pContentScale = parent->getContentBoxScale() - shrinkScaleBy;
    const auto& elements = parent->getElements();
    for (const auto& element : elements)
    {
        /* Shall not be taken into consideration for overflow */
        if (element->getTypeId() == uielements::UISlider::typeId
            && element->getCustomTagId() == uielements::UISlider::scrollTagId)
        { continue; }

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

auto BasicCalculator::calculateSlidersScaleAndPos(uielements::UIPane* parent) const -> glm::vec2
{
    glm::vec2 sliderImpact{0, 0};
    const auto& pComputedPos = parent->  getContentBoxPos();
    const auto& pComputedScale = parent->getContentBoxScale();
    if (const auto vSlider = parent->getVerticalSlider().lock(); vSlider && vSlider->isParented())
    {
        // Scroll sliders on a Pane can ONLY have PX values on the scroll direction.
        sliderImpact.x = vSlider->getScale().x.val;
        vSlider->setComputedPos({
            pComputedPos.x + pComputedScale.x - sliderImpact.x,
            pComputedPos.y
        });
        vSlider->setComputedScale({sliderImpact.x, pComputedScale.y});
    }

    if (const auto hSlider = parent->getHorizontalSlider().lock(); hSlider && hSlider->isParented())
    {
        // Scroll sliders on a Pane can ONLY have PX values on the scroll direction.
        sliderImpact.y = hSlider->getScale().y.val;
        hSlider->setComputedPos({
            pComputedPos.x,
            pComputedPos.y + pComputedScale.y - sliderImpact.y,
        });
        hSlider->setComputedScale({pComputedScale.x - sliderImpact.x, sliderImpact.y});
    }

    return sliderImpact;
}

auto BasicCalculator::calcPaneElementsAddScrollToPos(uielements::UIPane* parent,
    const glm::ivec2 offset) const -> void
{
    const auto& elements = parent->getElements();
    for (const auto& element : elements)
    {
        SKIP_SLIDER(element);
        element->setComputedPos(element->getComputedPos() - offset);
    }
}
} // namespace src::layoutcalculator
