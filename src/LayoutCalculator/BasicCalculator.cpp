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

#define SKIP_ABS_ELEMENT(element)\
    const auto& userPos = element->getPos();\
    if (userPos.x.type == LayoutBase::PositionType::ABS || userPos.y.type == LayoutBase::PositionType::ABS)\
    {\
        continue;\
    }\

auto BasicCalculator::get() -> BasicCalculator&
{
    static BasicCalculator instance;
    return instance;
}

auto BasicCalculator::calculateScaleForGenericElement(uielements::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> void
{
    const auto& elements = parent->getElements();
    if (elements.empty()) { return; }

    const auto& pType = parent->getType();
    if (pType == LayoutBase::Type::GRID)
    {
        return calculateScaleForGenericElementOfTypeGrid(parent, shrinkScaleBy);
    }

    /* Part of the parent area could be occupied by a scroll bar. We need to account for it. */
    const auto& pContentBoxScale = parent->getContentBoxScale() - shrinkScaleBy;

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
    const auto& elements = parent->getElements();
    if (elements.empty()) { return; }

    const auto& pType = parent->getType();
    if (pType == LayoutBase::Type::GRID)
    {
        return calculatePosForGenericElementOfTypeGrid(parent, shrinkScaleBy);
    }

    const auto& pContentBoxPos = parent->getContentBoxPos();
    const auto& pContentBoxScale = parent->getContentBoxScale() - shrinkScaleBy;
    const auto& pContentBoxMaxPoint = pContentBoxPos + pContentBoxScale;
    const auto& pWrap = parent->getWrap();

    SpacingDetails spacingDetails = calculateSpacingOnAxis(parent, shrinkScaleBy);

    glm::vec2 nextPos{pContentBoxPos + spacingDetails.additionalStartPush};
    glm::vec2 computedPos{0, 0};
    glm::vec2 maxOnAxis{0, 0};
    for (auto& element : elements)
    {
        SKIP_SLIDER(element);

        const auto& margins = element->getMargin();
        const auto& userPos = element->getPos();
        if (userPos.x.type == LayoutBase::PositionType::ABS || userPos.y.type == LayoutBase::PositionType::ABS)
        {
            element->setComputedPos({userPos.x.val, userPos.y.val});
            continue;
        }

        const auto& compScale = element->getComputedScale();
        const glm::vec2 fullBoxScale = element->getFullBoxScale();
        const glm::vec2 nextMaxPoint = nextPos + fullBoxScale;
        const glm::vec2 marginPush = glm::vec2{margins.left, margins.top};

        /* Note: `nextPos` starts at the end of the previous' element margin end. */
        if (pType == LayoutBase::Type::HORIZONTAL)
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
        else if (pType == LayoutBase::Type::VERTICAL)
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
        SKIP_ABS_ELEMENT(element);

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
        SKIP_ABS_ELEMENT(element);

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

auto BasicCalculator::calculateSplitPaneElements(uielements::UISplitPane* parent, const uint32_t handleIdx,
    const glm::vec2 mousePos) const -> void
{
    const auto handlesSize = calculateSplitPaneHandlesScale(parent);

    calculateSplitPaneNonHandleScale(parent, handlesSize);

    /* Mostly used when resizing the split pane. Otherwise the `if handle` function will do the constraint checks. */
    applySplitPaneElementsScaleCorrection(parent, handlesSize);

    calculateSplitPaneElementsPos(parent);

    if (handleIdx)
    {
        calculateSplitPaneRelativeValuesDueToDrag(parent, handleIdx, handlesSize, mousePos);
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

auto BasicCalculator::calculateScaleForGenericElementOfTypeGrid(uielements::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> void
{
    const auto& gridPolicy = parent->getGrid();
    if (gridPolicy.rows.empty() || gridPolicy.cols.empty()) { return; }

    /* Note for future: this precomputation doesn't need to be done each time, only when the grid policy changes. */
    calculatePrecomputedGridStartPos(parent, shrinkScaleBy);

    const auto& pContentScale = parent->getContentBoxScale()- shrinkScaleBy;
    const uint32_t nRow = gridPolicy.rows.size();
    const uint32_t nCol = gridPolicy.cols.size();
    const auto& elements = parent->getElements();
    for (auto& element : elements)
    {
        const auto& gridPos = element->getGridPos();
        const auto& gridSpan = element->getGridSpan();

        if (gridPos.col >= nCol) { continue; }
        if (gridPos.row >= nRow) { continue; }

        const glm::vec2 gridPosStart{
            gridPolicy.precompStart[gridPos.col],
            gridPolicy.precompStart[nCol + gridPos.row]
        };

        const glm::vec2 gridPosEnd{
            gridPos.col + gridSpan.col < nCol
                ? gridPolicy.precompStart[gridPos.col + gridSpan.col]
                : pContentScale.x,
            gridPos.row + gridSpan.row < nRow
                ? gridPolicy.precompStart[nCol + gridPos.row + gridSpan.row]
                : pContentScale.y,
        };

        element->setComputedScale(gridPosEnd - gridPosStart);
    }
}

auto BasicCalculator::calculatePosForGenericElementOfTypeGrid(uielements::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> void
{
    const auto& gridPolicy = parent->getGrid();
    const auto& elements = parent->getElements();
    const uint32_t nCol = gridPolicy.cols.size();
    for (auto& element : elements)
    {
        const auto& gridPos = element->getGridPos();
        element->setComputedPos(
            {
                gridPolicy.precompStart[gridPos.col],
                gridPolicy.precompStart[nCol + gridPos.row]
            });
    }
}

auto BasicCalculator::calculatePrecomputedGridStartPos(uielements::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> void
{
    /* Compute the amound of PX occupied space and FR parts in order to compute how much a FR part is worth. */
    auto& gridPolicy = parent->getGrid();
    glm::vec2 totalPx{0, 0};
    glm::vec2 totalFrac{0, 0};

    /* Columns "eat" space on X axis and rows on y axis. */
    for (const auto& col : gridPolicy.cols)
    {
        if (col.type == LayoutBase::ScaleType::PX) { totalPx.x += col.val; }
        if (col.type == LayoutBase::ScaleType::FR) { totalFrac.x += col.val; }
    }

    for (const auto& row : gridPolicy.rows)
    {
        if (row.type == LayoutBase::ScaleType::PX) { totalPx.y += row.val; }
        if (row.type == LayoutBase::ScaleType::FR) { totalFrac.y += row.val; }
    }

    const auto& pContentScale = parent->getContentBoxScale()- shrinkScaleBy;
    const float wFrac = (pContentScale.x - totalPx.x) / std::max(1.0f, totalFrac.x);
    const float hFrac = (pContentScale.y - totalPx.y) / std::max(1.0f, totalFrac.y);

    /* Fill the precomputed spots where each row/col starts. We only need to save the starts for one row and col.
    All the data is mashed in a flat array like this [cols_start.., rows_start..] */
    const uint32_t nRow = gridPolicy.rows.size();
    const uint32_t nCol = gridPolicy.cols.size();

    gridPolicy.precompStart.clear();
    gridPolicy.precompStart.reserve(nCol + nRow);

    glm::vec2 precompStart{parent->getContentBoxPos()};
    for (const auto& col : gridPolicy.cols)
    {
        gridPolicy.precompStart.push_back(std::round(precompStart.x));
        if (col.type == LayoutBase::ScaleType::PX) { precompStart.x += col.val; }
        if (col.type == LayoutBase::ScaleType::FR) { precompStart.x += col.val * wFrac; }
    }

    for (const auto& row : gridPolicy.rows)
    {
        gridPolicy.precompStart.push_back(std::round(precompStart.y));
        if (row.type == LayoutBase::ScaleType::PX) { precompStart.y += row.val; }
        if (row.type == LayoutBase::ScaleType::FR) { precompStart.y += row.val * hFrac; }
    }
}

auto BasicCalculator::calculateSplitPaneHandlesScale(uielements::UISplitPane* parent) const -> glm::vec2
{
    const auto& pLayoutType = parent->getType();
    const auto& pContentScale = parent->getContentBoxScale();
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
            cScale.y = pContentScale.y * userScale.y.val;
        }
        else if (pLayoutType == LayoutBase::Type::VERTICAL)
        {
            cScale.y = userScale.y.val;
            cScale.x = pContentScale.x * userScale.x.val;
        }

        handlesSize += cScale;
        element->setComputedScale(cScale);
    }

    return handlesSize;
}

auto BasicCalculator::calculateSplitPaneNonHandleScale(uielements::UISplitPane* parent,
    const glm::vec2 handlesSize) const -> void
{
    const auto& elements = parent->getElements();
    const auto& pLayoutType = parent->getType();
    const auto& pContentScale = parent->getContentBoxScale();
    const auto& reducedPContentScale = pContentScale - handlesSize;
    for (const auto& element : elements)
    {
        if (element->getTypeId() == uielements::UIButton::typeId) { continue; }

        const auto& userScale = element->getScale();
        glm::vec2 cScale;

        if (pLayoutType == LayoutBase::Type::HORIZONTAL)
        {
            cScale.x = reducedPContentScale.x * userScale.x.val;
            cScale.y = pContentScale.y * userScale.y.val;
            cScale.x = std::clamp(cScale.x, (float)element->getMinScale().x, (float)element->getMaxScale().x);
        }
        else if (pLayoutType == LayoutBase::Type::VERTICAL)
        {
            cScale.y = reducedPContentScale.y * userScale.y.val;
            cScale.x = pContentScale.x * userScale.x.val;
            cScale.y = std::clamp(cScale.y, (float)element->getMinScale().y, (float)element->getMaxScale().y);
        }

        element->setComputedScale(cScale);
    }
}

auto BasicCalculator::applySplitPaneElementsScaleCorrection(uielements::UISplitPane* parent,
    const glm::vec2 handlesSize) const -> void
{
    const auto& elements = parent->getElements();
    const auto& pContentScale = parent->getContentBoxScale();
    const auto& reducedPContentScale = pContentScale - handlesSize;
    for (int32_t handleIdx = 0; handleIdx < (int32_t)elements.size() - 1; ++handleIdx)
    {
        if (elements[handleIdx]->getTypeId() != uielements::UIButton::typeId) { continue; }

        glm::vec2 wantedOffsetRel{0, 0};

        const uint32_t lPaneIdx = handleIdx - 1;
        const uint32_t rPaneIdx = handleIdx + 1;
        const glm::vec2 lpMinScaleRel = elements[lPaneIdx]->getMinScale() / reducedPContentScale;
        const glm::vec2 lpMaxScaleRel = elements[lPaneIdx]->getMaxScale() / reducedPContentScale;
        const glm::vec2 rpMinScaleRel = elements[rPaneIdx]->getMinScale() / reducedPContentScale;
        const glm::vec2 rpMaxScaleRel = elements[rPaneIdx]->getMaxScale() / reducedPContentScale;

        auto lScale = elements[lPaneIdx]->getScale();
        auto rScale = elements[rPaneIdx]->getScale();
        if (parent->getType() == LayoutBase::Type::HORIZONTAL)
        {
            if (lScale.x.val < lpMinScaleRel.x)
            {
                wantedOffsetRel.x = lpMinScaleRel.x - lScale.x.val;
            }

            if (rScale.x.val < rpMinScaleRel.x)
            {
                wantedOffsetRel.x = rScale.x.val - rpMinScaleRel.x;
            }

            if (lScale.x.val > lpMaxScaleRel.x)
            {
                wantedOffsetRel.x = lpMaxScaleRel.x - lScale.x.val;
            }

            if (rScale.x.val > rpMaxScaleRel.x)
            {
                wantedOffsetRel.x = rScale.x.val - rpMaxScaleRel.x;
            }

            /* Apply the relative offsets correction. */
            lScale.x.val += wantedOffsetRel.x;
            elements[lPaneIdx]->setScale(lScale);

            rScale.x.val -= wantedOffsetRel.x;
            elements[rPaneIdx]->setScale(rScale);

            auto lCompScale = elements[lPaneIdx]->getComputedScale();
            lCompScale.x = lScale.x.val * reducedPContentScale.x;
            elements[lPaneIdx]->setComputedScale(lCompScale);

            auto rCompScale = elements[rPaneIdx]->getComputedScale();
            rCompScale.x = rScale.x.val * reducedPContentScale.x;
            elements[rPaneIdx]->setComputedScale(rCompScale);
        }
        else if (parent->getType() == LayoutBase::Type::VERTICAL)
        {
            if (lScale.y.val < lpMinScaleRel.y)
            {
                wantedOffsetRel.y = lpMinScaleRel.y - lScale.y.val;
            }

            if (rScale.y.val < rpMinScaleRel.y)
            {
                wantedOffsetRel.y = rScale.y.val - rpMinScaleRel.y;
            }

            if (lScale.y.val > lpMaxScaleRel.y)
            {
                wantedOffsetRel.y = lpMaxScaleRel.y - lScale.y.val;
            }

            if (rScale.y.val > rpMaxScaleRel.y)
            {
                wantedOffsetRel.y = rScale.y.val - rpMaxScaleRel.y;
            }

            /* Apply the relative offsets correction. */
            lScale.y.val += wantedOffsetRel.y;
            elements[lPaneIdx]->setScale(lScale);

            rScale.y.val -= wantedOffsetRel.y;
            elements[rPaneIdx]->setScale(rScale);

            auto lCompScale = elements[lPaneIdx]->getComputedScale();
            lCompScale.y = lScale.y.val * reducedPContentScale.y;
            elements[lPaneIdx]->setComputedScale(lCompScale);

            auto rCompScale = elements[rPaneIdx]->getComputedScale();
            rCompScale.y = rScale.y.val * reducedPContentScale.y;
            elements[rPaneIdx]->setComputedScale(rCompScale);
        }
    }
}

auto BasicCalculator::calculateSplitPaneElementsPos(uielements::UISplitPane* parent) const -> void
{
    const auto& elements = parent->getElements();
    const auto& pLayoutType = parent->getType();
    const auto& pContentPos = parent->getContentBoxPos();
    glm::vec2 nextPos{pContentPos};
    glm::vec2 pos{0, 0};
    for (auto& element : elements)
    {
        const auto& margins = element->getMargin();
        const auto& compScale = element->getComputedScale();
        if (pLayoutType == LayoutBase::Type::HORIZONTAL)
        {
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

auto BasicCalculator::calculateSplitPaneRelativeValuesDueToDrag(uielements::UISplitPane* parent,
    const uint32_t handleIdx, const glm::vec2 handlesSize, const glm::vec2 mousePos) const -> void
{
    const auto& elements = parent->getElements();
    const auto& pContentBoxScale = parent->getContentBoxScale();
    const auto& pLayoutType = parent->getType();

    /* Calculate difference between the current mouse position and the handle's center. */
    const auto handleCenter = elements[handleIdx]->getComputedPos()
        + elements[handleIdx]->getComputedScale() * 0.5f;
    const glm::vec2 mouseDiff = mousePos - handleCenter;

    const uint32_t lPaneIdx = handleIdx - 1;
    const uint32_t rPaneIdx = handleIdx + 1;

    const auto& contentScale = pContentBoxScale - handlesSize;

    glm::vec2 wantedOffsetRel = mouseDiff / contentScale;

    const glm::vec2 lpMinScaleRel = elements[lPaneIdx]->getMinScale() / contentScale;
    const glm::vec2 lpMaxScaleRel = elements[lPaneIdx]->getMaxScale() / contentScale;
    const glm::vec2 rpMinScaleRel = elements[rPaneIdx]->getMinScale() / contentScale;
    const glm::vec2 rpMaxScaleRel = elements[rPaneIdx]->getMaxScale() / contentScale;

    /* Add the maximum amount of offset to the two panes without violating any constraints. */
    auto lScale = elements[lPaneIdx]->getScale();
    auto rScale = elements[rPaneIdx]->getScale();
    if (pLayoutType == LayoutBase::Type::HORIZONTAL)
    {
        wantedOffsetRel.x = constrainOffset(wantedOffsetRel.x,
            lScale.x.val, lpMinScaleRel.x, lpMaxScaleRel.x,
            rScale.x.val, rpMinScaleRel.x, rpMaxScaleRel.x);

        /* Apply the relative offsets. */
        lScale.x.val += wantedOffsetRel.x;
        elements[lPaneIdx]->setScale(lScale);

        rScale.x.val -= wantedOffsetRel.x;
        elements[rPaneIdx]->setScale(rScale);
    }
    else if (pLayoutType == LayoutBase::Type::VERTICAL)
    {
        wantedOffsetRel.y = constrainOffset(wantedOffsetRel.y,
            lScale.y.val, lpMinScaleRel.y, lpMaxScaleRel.y,
            rScale.y.val, rpMinScaleRel.y, rpMaxScaleRel.y);

        /* Apply the relative offsets. */
        lScale.y.val += wantedOffsetRel.y;
        elements[lPaneIdx]->setScale(lScale);

        rScale.y.val -= wantedOffsetRel.y;
        elements[rPaneIdx]->setScale(rScale);
    }
}

auto BasicCalculator::constrainOffset(float wantedOffset,
    const float lpScale, const float lpMin, const float lpMax,
    const float rpScale, const float rpMin, const float rpMax) const -> float
{
    /* If left pane will go under min scale by adding the offset then cap the wanted
        offset to the maximum possible value to add. */
    if (lpScale + wantedOffset < lpMin)
    {
        wantedOffset = lpMin - lpScale;
    }

    /* If right pane will go under min scale by subtracting the offset then cap the wanted
        offset to the maximum possible value to subtract. */
    if (rpScale - wantedOffset < rpMin)
    {
        wantedOffset = rpScale - rpMin;
    }

    /* If left pane will go above max scale by adding the offset then cap the wanted
        offset to the maximum possible value to add. */
    if (lpScale + wantedOffset > lpMax)
    {
        wantedOffset = lpMax - lpScale;
    }

    /* If right pane will go above max scale by subtracting the offset then cap the wanted
        offset to the maximum possible value to subtract. */
    if (rpScale - wantedOffset > rpMax)
    {
        wantedOffset = rpScale - rpMax;
    }

    return wantedOffset;
}

} // namespace src::layoutcalculator
