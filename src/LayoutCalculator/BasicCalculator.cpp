#include "BasicCalculator.hpp"
#include "src/ElementComposable/LayoutBase.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UISlider.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

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

auto BasicCalculator::calculateScaleForGenericElement(uielements::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> void
{
    /* Part of the parent area could be occupied by a scroll bar. We need to account for it. */
    const auto& pContentBoxScale = parent->getContentBoxScale() - shrinkScaleBy;
    const auto& elements = parent->getElements();

    glm::vec2 nonFillRunningTotal{0, 0};
    glm::vec2 fillsNeededPerAxis{0, 0};
    glm::vec2 cScale{0, 0};
    for (const auto& element : elements)
    {
        SKIP_SLIDER(element);

        const auto& userScale = element->getScale();
        const auto& marginTB = element->getTBMargin();
        const auto& marginLR = element->getLRMargin();
        const bool isXPx = userScale.x.type == LayoutBase::ScaleType::PX;
        const bool isYPx = userScale.y.type == LayoutBase::ScaleType::PX;
        const bool isXRel = userScale.x.type == LayoutBase::ScaleType::REL;
        const bool isYRel = userScale.y.type == LayoutBase::ScaleType::REL;
        const bool isXFill = userScale.x.type == LayoutBase::ScaleType::FILL;
        const bool isYFill = userScale.y.type == LayoutBase::ScaleType::FILL;
        const bool isXFit = userScale.x.type == LayoutBase::ScaleType::FIT;
        const bool isYFit = userScale.y.type == LayoutBase::ScaleType::FIT;

        if (isXPx) { cScale.x = userScale.x.val - marginLR; }
        else if (isXRel) { cScale.x = pContentBoxScale.x * userScale.x.val - marginLR; }
        else if (isXFill) { ++fillsNeededPerAxis.x; }

        if (isYPx) { cScale.y = userScale.y.val - marginTB; }
        else if (isYRel) { cScale.y = pContentBoxScale.y * userScale.y.val - marginTB; }
        else if (isYFill) { ++fillsNeededPerAxis.y; }

        if (isXFit || isYFit)
        {
            /* Returns a mock `userScale` like value calculating what scale the element would be
                if it wrapped all it's children tightly. */
            const glm::vec2 fitScale = calculateFitScale(element.get());
            if (isXFit) { cScale.x = fitScale.x - marginLR; }
            if (isYFit) { cScale.y = fitScale.y - marginTB; }
        }

        element->setComputedScale(cScale);

        nonFillRunningTotal += cScale;
    }

    /* Process the FILL nodes. */
    const glm::vec2 equalFillSpace = (pContentBoxScale - nonFillRunningTotal) / fillsNeededPerAxis;
    for (const auto& element : elements)
    {
        SKIP_SLIDER(element);

        const auto& userScale = element->getScale();
        const bool isXFill = userScale.x.type == LayoutBase::ScaleType::FILL;
        const bool isYFill = userScale.y.type == LayoutBase::ScaleType::FILL;

        if (!isXFill && !isYFill) { continue; }

        const auto& marginTB = element->getTBMargin();
        const auto& marginLR = element->getLRMargin();
        cScale = element->getComputedScale();
        if (isXFill)
        {
            cScale.x = equalFillSpace.x - marginLR;
        }

        if (isYFill)
        {
            cScale.y = equalFillSpace.y - marginTB;
        }

        element->setComputedScale(cScale);
    }
}

auto BasicCalculator::calculatePositionForGenericElement(uielements::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> void
{
    const auto& pContentBoxPos = parent->getContentBoxPos();
    const auto& pContentBoxScale = parent->getContentBoxScale() - shrinkScaleBy;
    const auto& pContentBoxMaxPoint = pContentBoxPos + pContentBoxScale;
    const auto& pLayoutType = parent->getType();
    const auto& pWrap = parent->getWrap();
    const auto& elements = parent->getElements();

    SpacingDetails spacingDetails = calculateSpacingOnAxis(parent, shrinkScaleBy);

    glm::vec2 nextPos{pContentBoxPos + spacingDetails.additionalStartPush};
    glm::vec2 computedPos{0, 0};
    glm::vec2 maxOnAxis{0, 0};
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
            nextPos.x = computedPos.x + compScale.x + margins.right + spacingDetails.spaceBetween.x;
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
            nextPos.y = computedPos.y + compScale.y + margins.bot + spacingDetails.spaceBetween.y;
        }
        maxOnAxis = utils::max(maxOnAxis, fullBoxScale);

        element->setComputedPos(computedPos);
    }
}

auto BasicCalculator::calculateAlignmentForElements(uielements::UIBase* parent,
    const glm::vec2 overflow) const -> void
{
    if (overflow.x >= 0 && overflow.y >= 0) { return; }

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
}

auto BasicCalculator::calculateElementOverflow(uielements::UIBase* parent,
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

auto BasicCalculator::calculateSpacingOnAxis(uielements::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> SpacingDetails
{
    const auto& elements = parent->getElements();
    const auto& pLayoutType = parent->getType();
    const auto& pSpacing = parent->getSpacing();
    const auto& pContentBoxScale = parent->getContentBoxScale();

    glm::vec2 computedSpacing{0, 0};
    glm::vec2 additionalStartPush{0,};

    /* Calculate max running scale and valid elements for spacing calculations. */
    glm::vec2 maxRunningScale{shrinkScaleBy};
    int32_t elementCountForSpacing{0};
    for (const auto& element : elements)
    {
        SKIP_SLIDER(element);
        maxRunningScale += element->getFullBoxScale();
        ++elementCountForSpacing;
    }

    glm::vec2 nextPos{parent->getContentBoxPos()};
    glm::vec2 computedPos{0, 0};
    glm::vec2 maxOnAxis{0, 0};

    switch (pSpacing)
    {
        case LayoutBase::Spacing::TIGHT:
            // Do nothing
            break;
        case LayoutBase::Spacing::EVEN_NO_GAP:
            computedSpacing = (pContentBoxScale - maxRunningScale) / (elementCountForSpacing - 1);
            computedSpacing = utils::max({0, 0}, computedSpacing);
            break;
        case LayoutBase::Spacing::EVEN_GAP:
            computedSpacing = (pContentBoxScale - maxRunningScale) / elementCountForSpacing;
            computedSpacing = utils::max({0, 0}, computedSpacing);
            additionalStartPush += computedSpacing * 0.5f;
            break;
    }

    if (pLayoutType == LayoutBase::Type::HORIZONTAL)
    {
        computedSpacing.y = 0;
        additionalStartPush.y = 0;
        return SpacingDetails{additionalStartPush, computedSpacing};
    }
    else if (pLayoutType == LayoutBase::Type::VERTICAL)
    {
        computedSpacing.x = 0;
        additionalStartPush.x = 0;
        return SpacingDetails{additionalStartPush, computedSpacing};
    }

    return SpacingDetails{};
}

auto BasicCalculator::calcSplitPaneElements(uielements::UISplitPane* parent) const -> void
{
    const auto& pLayoutType = parent->getType();
    const auto& pComputedScale = parent->getComputedScale();
    const auto& elements = parent->getElements();

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

auto BasicCalculator::calculateElementsOffsetDueToScroll(uielements::UIPane* parent,
    const glm::ivec2 offset) const -> void
{
    const auto& elements = parent->getElements();
    for (const auto& element : elements)
    {
        SKIP_SLIDER(element);
        element->setComputedPos(element->getComputedPos() - offset);
    }
}

auto BasicCalculator::calculateFitScale(uielements::UIBase* parent) const -> glm::vec2
{
    const auto& elements = parent->getElements();
    if (elements.empty())
    {
        utils::Logger("calc").warn("no elements for fit");
        return {0, 0};
    }

    const auto& pMarginLR = parent->getLRMargin();
    const auto& pMarginTB = parent->getTBMargin();
    const auto& pBorder = parent->getBorder();
    const auto& pPadding = parent->getPadding();
    const auto& pType = parent->getType();
    const auto& pUserScale = parent->getScale();
    const bool pIsXFit = pUserScale.x.type == LayoutBase::ScaleType::FIT;
    const bool pIsYFit = pUserScale.y.type == LayoutBase::ScaleType::FIT;
    const bool pIsHori = pType == LayoutBase::Type::HORIZONTAL;
    const bool pIsVert = pType == LayoutBase::Type::VERTICAL;

    glm::vec2 fitScale{0, 0};
    for (const auto& element : elements)
    {
        SKIP_SLIDER(element);
        const auto& userScale = element->getScale();
        const bool isXPx = userScale.x.type == LayoutBase::ScaleType::PX;
        const bool isYPx = userScale.y.type == LayoutBase::ScaleType::PX;
        const bool isXFit = userScale.x.type == LayoutBase::ScaleType::FIT;
        const bool isYFit = userScale.y.type == LayoutBase::ScaleType::FIT;

        glm::vec2 internalFitScale{0, 0};
        if (isXFit || isYFit)
        {
            internalFitScale = calculateFitScale(element.get());
        }

        float value{0};
        if (pIsXFit)
        {
            if (isXPx) { value = userScale.x.val; }
            else if (isXFit) { value = internalFitScale.x; }
            fitScale.x = pIsVert ? std::max(fitScale.x, value) : value + fitScale.x;
        }

        if (pIsYFit)
        {
            if (isYPx) { value = userScale.y.val; }
            else if (isYFit) { value = internalFitScale.y; }
            fitScale.y = pIsHori ? std::max(fitScale.y, value) : value + fitScale.y;
        }
    }

    /* Adjust for border and padding of the parent */
    fitScale.x += pBorder.left + pBorder.right + pPadding.left + pPadding.right + pMarginLR;
    fitScale.y += pBorder.top + pBorder.bot + pPadding.top + pPadding.bot + pMarginTB;
    return fitScale;
}

} // namespace src::layoutcalculator
