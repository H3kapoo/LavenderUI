#include "BasicCalculator.hpp"
#include "src/ElementComposable/LayoutBase.hpp"
#include "src/UINodes/UIBase.hpp"
#include "src/UINodes/UIButton.hpp"
// #include "src/Uinodes/UIDropdown.hpp"
// #include "src/Uinodes/UIPane.hpp"
// #include "src/Uinodes/UISlider.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

namespace src::layoutcalculator
{
using namespace elementcomposable;

#define SKIP_SLIDER(element)\
    // if (element->getTypeId() == uinodes::UISlider::typeId\
    // && element->getCustomTagId() == uinodes::UISlider::scrollTagId)\
    // { continue; }\

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

auto BasicCalculator::calculateScaleForGenericElement(uinodes::UIBase* node,
    const glm::vec2 shrinkScaleBy) const -> void
{
    const auto& childNodes = node->getElements();
    if (childNodes.empty()) { return; }

    const auto& nLayout = node->getBaseLayoutData();
    const auto& nType = nLayout.getType();
    if (nType == LayoutBase::Type::GRID)
    {
        return calculateScaleForGenericElementOfTypeGrid(node, shrinkScaleBy);
    }

    /* Part of the parent area could be occupied by a scroll bar. We need to account for it. */
    const auto& nContentBoxScale = nLayout.getContentBoxScale() - shrinkScaleBy;

    glm::vec2 nonFillRunningTotal{0, 0};
    glm::vec2 fillsNeededPerAxis{0, 0};
    glm::vec2 cScale{0, 0};
    for (const auto& childNode : childNodes)
    {
        SKIP_SLIDER(childNode);

        auto& chLayout = childNode->getBaseLayoutData();
        const auto& userScale = chLayout.getScale();
        const auto& marginTB = chLayout.getTBMargin();
        const auto& marginLR = chLayout.getLRMargin();
        const bool isXPx = userScale.x.type == LayoutBase::ScaleType::PX;
        const bool isYPx = userScale.y.type == LayoutBase::ScaleType::PX;
        const bool isXRel = userScale.x.type == LayoutBase::ScaleType::REL;
        const bool isYRel = userScale.y.type == LayoutBase::ScaleType::REL;
        const bool isXFill = userScale.x.type == LayoutBase::ScaleType::FILL;
        const bool isYFill = userScale.y.type == LayoutBase::ScaleType::FILL;
        const bool isXFit = userScale.x.type == LayoutBase::ScaleType::FIT;
        const bool isYFit = userScale.y.type == LayoutBase::ScaleType::FIT;

        if (isXPx) { cScale.x = userScale.x.val - marginLR; }
        else if (isXRel) { cScale.x = nContentBoxScale.x * userScale.x.val - marginLR; }
        else if (isXFill) { ++fillsNeededPerAxis.x; }

        if (isYPx) { cScale.y = userScale.y.val - marginTB; }
        else if (isYRel) { cScale.y = nContentBoxScale.y * userScale.y.val - marginTB; }
        else if (isYFill) { ++fillsNeededPerAxis.y; }

        if (isXFit || isYFit)
        {
            /* Returns a mock `userScale` like value calculating what scale the element would be
                if it wrapped all it's children tightly. */
            const glm::vec2 fitScale = calculateFitScale(childNode.get());
            if (isXFit) { cScale.x = fitScale.x - marginLR; }
            if (isYFit) { cScale.y = fitScale.y - marginTB; }
        }

        chLayout.setComputedScale(cScale);

        nonFillRunningTotal += cScale;
    }

    /* Process the FILL nodes. */
    const glm::vec2 equalFillSpace = (nContentBoxScale - nonFillRunningTotal) / fillsNeededPerAxis;
    for (const auto& childNode : childNodes)
    {
        SKIP_SLIDER(childNode);

        auto& chLayout = childNode->getBaseLayoutData();
        const auto& userScale = chLayout.getScale();
        const bool isXFill = userScale.x.type == LayoutBase::ScaleType::FILL;
        const bool isYFill = userScale.y.type == LayoutBase::ScaleType::FILL;

        if (!isXFill && !isYFill) { continue; }

        const auto& marginTB = chLayout.getTBMargin();
        const auto& marginLR = chLayout.getLRMargin();
        cScale = chLayout.getComputedScale();
        if (isXFill)
        {
            cScale.x = equalFillSpace.x - marginLR;
        }

        if (isYFill)
        {
            cScale.y = equalFillSpace.y - marginTB;
        }

        chLayout.setComputedScale(cScale);
    }
}

auto BasicCalculator::calculatePositionForGenericElement(uinodes::UIBase* node,
    const glm::vec2 shrinkScaleBy) const -> void
{
    const auto& childNodes = node->getElements();
    if (childNodes.empty()) { return; }

    const auto& nLayout = node->getBaseLayoutData();
    const auto& nType = nLayout.getType();
    if (nType == LayoutBase::Type::GRID)
    {
        return calculatePosForGenericElementOfTypeGrid(node, shrinkScaleBy);
    }

    const auto& nContentBoxPos = nLayout.getContentBoxPos();
    const auto& nContentBoxScale = nLayout.getContentBoxScale() - shrinkScaleBy;
    const auto& nContentBoxMaxPoint = nContentBoxPos + nContentBoxScale;
    const auto& nWrap = nLayout.getWrap();

    SpacingDetails spacingDetails = calculateSpacingOnAxis(node, shrinkScaleBy);

    glm::vec2 nextPos{nContentBoxPos + spacingDetails.additionalStartPush};
    glm::vec2 computedPos{0, 0};
    glm::vec2 maxOnAxis{0, 0};
    for (auto& childNode : childNodes)
    {
        SKIP_SLIDER(childNode);

        auto& chLayout = childNode->getBaseLayoutData();
        const auto& margins = chLayout.getMargin();
        const auto& userPos = chLayout.getPos();
        if (userPos.x.type == LayoutBase::PositionType::ABS || userPos.y.type == LayoutBase::PositionType::ABS)
        {
            // TODO: Take into consideration element's margins.
            // Margins will push the computed position inwards.
            chLayout.setComputedPos({userPos.x.val, userPos.y.val});
            continue;
        }

        const auto& compScale = chLayout.getComputedScale();
        const glm::vec2 fullBoxScale = chLayout.getFullBoxScale();
        const glm::vec2 nextMaxPoint = nextPos + fullBoxScale;
        const glm::vec2 marginPush = glm::vec2{margins.left, margins.top};

        /* Note: `nextPos` starts at the end of the previous' element margin end. */
        if (nType == LayoutBase::Type::HORIZONTAL)
        {
            if (nWrap && nextMaxPoint.x > nContentBoxMaxPoint.x)
            {
                nextPos.y += maxOnAxis.y;
                nextPos.x = nContentBoxPos.x;
                maxOnAxis.y = 0;
            }

            computedPos = nextPos + marginPush;
            nextPos.x = computedPos.x + compScale.x + margins.right + spacingDetails.spaceBetween.x;
        }
        else if (nType == LayoutBase::Type::VERTICAL)
        {
            if (nWrap && nextMaxPoint.y > nContentBoxMaxPoint.y)
            {
                nextPos.x += maxOnAxis.x;
                nextPos.y = nContentBoxPos.y;
                maxOnAxis.x = 0;
            }

            computedPos = nextPos + marginPush;
            nextPos.y = computedPos.y + compScale.y + margins.bot + spacingDetails.spaceBetween.y;
        }
        maxOnAxis = utils::max(maxOnAxis, fullBoxScale);

        chLayout.setComputedPos(computedPos);
    }
}

auto BasicCalculator::calculateAlignmentForElements(uinodes::UIBase* node,
    const glm::vec2 overflow) const -> void
{
    if (overflow.x >= 0 && overflow.y >= 0) { return; }

    /* Note: negative overflow means there's `-overflow` pixels left until an overflow occurs.
        We can leverage that to align elements.*/
    const auto& childNodes = node->getElements();
    const auto& nLayout = node->getBaseLayoutData();
    const auto& nAlign = nLayout.getAlign();
    const auto& nType = nLayout.getType();
    const auto& isTightSpacing = nLayout.getSpacing() == LayoutBase::Spacing::TIGHT;
    for (auto& childNode : childNodes)
    {
        SKIP_SLIDER(childNode);
        // SKIP_ABS_ELEMENT(childNode);

        glm::vec2 offset{0, 0};
        switch (nAlign)
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

        if (nType == LayoutBase::Type::HORIZONTAL && !isTightSpacing)
        {
            offset.x = 0;
        }
        else if (nType == LayoutBase::Type::VERTICAL && !isTightSpacing)
        {
            offset.y = 0;
        }

        auto& chLayout = childNode->getBaseLayoutData();
        chLayout.setComputedPos(chLayout.getComputedPos() + offset);
    }
}

auto BasicCalculator::calculateElementOverflow(uinodes::UIBase* node,
    const glm::vec2 shrinkScaleBy) const -> glm::vec2
{
    glm::vec2 boxScale{0, 0};
    const auto& childNodes = node->getElements();
    const auto& nLayout = node->getBaseLayoutData();
    const auto& pContentPos = nLayout.getContentBoxPos();
    const auto& pContentScale = nLayout.getContentBoxScale() - shrinkScaleBy;
    for (const auto& childNode : childNodes)
    {
        // SKIP_ABS_ELEMENT(element);

        /* Shall not be taken into consideration for overflow */
        SKIP_SLIDER(childNode);

        const auto& chLayout = childNode->getBaseLayoutData();
        const auto& fullPos = chLayout.getFullBoxPos();
        const auto& fullScale = chLayout.getFullBoxScale();
        boxScale.x = std::max(boxScale.x, fullPos.x + fullScale.x);
        boxScale.y = std::max(boxScale.y, fullPos.y + fullScale.y);
    }

    return boxScale - (pContentPos + pContentScale);
}

auto BasicCalculator::calculateSpacingOnAxis(uinodes::UIBase* node,
    const glm::vec2 shrinkScaleBy) const -> SpacingDetails
{
    const auto& childNodes = node->getElements();
    const auto& nLayout = node->getBaseLayoutData();
    const auto& nLayoutType = nLayout.getType();
    const auto& nSpacing = nLayout.getSpacing();
    const auto& nContentBoxScale = nLayout.getContentBoxScale();

    glm::vec2 computedSpacing{0, 0};
    glm::vec2 additionalStartPush{0,};

    /* Calculate max running scale and valid elements for spacing calculations. */
    glm::vec2 maxRunningScale{shrinkScaleBy};
    int32_t elementCountForSpacing{0};
    for (const auto& childNode : childNodes)
    {
        SKIP_SLIDER(element);
        maxRunningScale += childNode->getBaseLayoutData().getFullBoxScale();
        ++elementCountForSpacing;
    }

    glm::vec2 nextPos{nLayout.getContentBoxPos()};
    glm::vec2 computedPos{0, 0};
    glm::vec2 maxOnAxis{0, 0};

    switch (nSpacing)
    {
        case LayoutBase::Spacing::TIGHT:
            // Do nothing
            break;
        case LayoutBase::Spacing::EVEN_NO_GAP:
            computedSpacing = (nContentBoxScale - maxRunningScale) / (elementCountForSpacing - 1);
            computedSpacing = utils::max({0, 0}, computedSpacing);
            break;
        case LayoutBase::Spacing::EVEN_GAP:
            computedSpacing = (nContentBoxScale - maxRunningScale) / elementCountForSpacing;
            computedSpacing = utils::max({0, 0}, computedSpacing);
            additionalStartPush += computedSpacing * 0.5f;
            break;
    }

    if (nLayoutType == LayoutBase::Type::HORIZONTAL)
    {
        computedSpacing.y = 0;
        additionalStartPush.y = 0;
        return SpacingDetails{additionalStartPush, computedSpacing};
    }
    else if (nLayoutType == LayoutBase::Type::VERTICAL)
    {
        computedSpacing.x = 0;
        additionalStartPush.x = 0;
        return SpacingDetails{additionalStartPush, computedSpacing};
    }

    return SpacingDetails{};
}

// auto BasicCalculator::calculateSplitPaneElements(uinodes::UISplitPane* parent, const uint32_t handleIdx,
//     const glm::vec2 mousePos) const -> void
// {
//     const auto handlesSize = calculateSplitPaneHandlesScale(parent);

//     calculateSplitPaneNonHandleScale(parent, handlesSize);

//     /* Mostly used when resizing the split pane. Otherwise the `if handle` function will do the constraint checks. */
//     applySplitPaneElementsScaleCorrection(parent, handlesSize);

//     calculateSplitPaneElementsPos(parent);

//     if (handleIdx)
//     {
//         calculateSplitPaneRelativeValuesDueToDrag(parent, handleIdx, handlesSize, mousePos);
//     }
// }

// auto BasicCalculator::calculatePositionForDropdownElement(uinodes::UIDropdown* dropdown) const -> void
// {
//     if (dropdown->getElements().empty()) { return; }

//     const auto gp = dropdown->getGrandParent().lock();
//     const bool isNestedDropdown = gp != nullptr && gp->getTypeId() == uinodes::UIDropdown::typeId;

//     /* If this dropdown is nested, refer to bounds of the container Pane.
//         Otherwise refer to bounds of the dropdown itself. */
//     const auto& optionsHolder = dropdown->getElements().at(0);
//     const auto openDir = dropdown->getOpenDirection();
//     const glm::ivec2 ddBoxPos = dropdown->getFullBoxPos();
//     const glm::ivec2 ddBoxScale = dropdown->getFullBoxScale();

//     glm::ivec2 hostPanePos{0, 0};
//     glm::ivec2 hostPaneScale{0, 0};
//     if (isNestedDropdown)
//     {
//         const auto ddHostPane = dropdown->getParent().lock();
//         hostPanePos = ddHostPane->getFullBoxPos();
//         hostPaneScale = ddHostPane->getFullBoxScale();
//     }

//     LayoutBase::PositionXY finalPos{{0, LayoutBase::PositionType::ABS}, {0, LayoutBase::PositionType::ABS}};
//     switch (openDir)
//     {
//         case uinodes::UIDropdown::OpenDir::TOP:
//         {
//             finalPos.x.val = ddBoxPos.x;
//             finalPos.y.val = ddBoxPos.y - optionsHolder->getFullBoxScale().y;
//             break;
//         }
//         case uinodes::UIDropdown::OpenDir::BOTTOM:
//         {
//             finalPos.x.val = ddBoxPos.x;
//             finalPos.y.val = ddBoxPos.y + ddBoxScale.y;
//             break;
//         }
//         case uinodes::UIDropdown::OpenDir::LEFT:
//         {
//             finalPos.x.val = isNestedDropdown ? hostPanePos.x - hostPaneScale.x : ddBoxPos.x + ddBoxScale.x;
//             finalPos.y.val = ddBoxPos.y;
//             break;
//         }
//         case uinodes::UIDropdown::OpenDir::RIGHT:
//         {
//             finalPos.x.val = isNestedDropdown ? hostPanePos.x + hostPaneScale.x : ddBoxPos.x + ddBoxScale.x;
//             finalPos.y.val = ddBoxPos.y;
//             break;
//         }
//     }

//     optionsHolder->setPos(finalPos);

//     calculatePositionForGenericElement(dropdown);
// }

// auto BasicCalculator::calculateSlidersScaleAndPos(uinodes::UIPane* parent) const -> glm::vec2
// {
//     glm::vec2 sliderImpact{0, 0};
//     const auto& pComputedPos = parent->  getContentBoxPos();
//     const auto& pComputedScale = parent->getContentBoxScale();
//     if (const auto vSlider = parent->getVerticalSlider().lock(); vSlider && vSlider->isParented())
//     {
//         // Scroll sliders on a Pane can ONLY have PX values on the scroll direction.
//         sliderImpact.x = vSlider->getScale().x.val;
//         vSlider->setComputedPos({
//             pComputedPos.x + pComputedScale.x - sliderImpact.x,
//             pComputedPos.y
//         });
//         vSlider->setComputedScale({sliderImpact.x, pComputedScale.y});
//     }

//     if (const auto hSlider = parent->getHorizontalSlider().lock(); hSlider && hSlider->isParented())
//     {
//         // Scroll sliders on a Pane can ONLY have PX values on the scroll direction.
//         sliderImpact.y = hSlider->getScale().y.val;
//         hSlider->setComputedPos({
//             pComputedPos.x,
//             pComputedPos.y + pComputedScale.y - sliderImpact.y,
//         });
//         hSlider->setComputedScale({pComputedScale.x - sliderImpact.x, sliderImpact.y});
//     }

//     return sliderImpact;
// }

// auto BasicCalculator::calculateElementsOffsetDueToScroll(uinodes::UIPane* parent,
//     const glm::ivec2 offset) const -> void
// {
//     const auto& elements = parent->getElements();
//     for (const auto& element : elements)
//     {
//         SKIP_SLIDER(element);
//         element->setComputedPos(element->getComputedPos() - offset);
//     }
// }

auto BasicCalculator::calculateFitScale(uinodes::UIBase* node) const -> glm::vec2
{
    const auto& childNodes = node->getElements();
    if (childNodes.empty())
    {
        utils::Logger("calc").warn("no elements for fit");
        return {0, 0};
    }

    const auto& nLayout = node->getBaseLayoutData();
    const auto& nMarginLR = nLayout.getLRMargin();
    const auto& nMarginTB = nLayout.getTBMargin();
    const auto& nBorder = nLayout.getBorder();
    const auto& nPadding = nLayout.getPadding();
    const auto& nType = nLayout.getType();
    const auto& nUserScale = nLayout.getScale();
    const bool nIsXFit = nUserScale.x.type == LayoutBase::ScaleType::FIT;
    const bool nIsYFit = nUserScale.y.type == LayoutBase::ScaleType::FIT;
    const bool nIsHori = nType == LayoutBase::Type::HORIZONTAL;
    const bool nIsVert = nType == LayoutBase::Type::VERTICAL;

    glm::vec2 fitScale{0, 0};
    for (const auto& childNode : childNodes)
    {
        SKIP_SLIDER(element);
        const auto& userScale = childNode->getBaseLayoutData().getScale();
        const bool isXPx = userScale.x.type == LayoutBase::ScaleType::PX;
        const bool isYPx = userScale.y.type == LayoutBase::ScaleType::PX;
        const bool isXFit = userScale.x.type == LayoutBase::ScaleType::FIT;
        const bool isYFit = userScale.y.type == LayoutBase::ScaleType::FIT;

        glm::vec2 internalFitScale{0, 0};
        if (isXFit || isYFit)
        {
            internalFitScale = calculateFitScale(childNode.get());
        }

        float value{0};
        if (nIsXFit)
        {
            if (isXPx) { value = userScale.x.val; }
            else if (isXFit) { value = internalFitScale.x; }
            fitScale.x = nIsVert ? std::max(fitScale.x, value) : value + fitScale.x;
        }

        if (nIsYFit)
        {
            if (isYPx) { value = userScale.y.val; }
            else if (isYFit) { value = internalFitScale.y; }
            fitScale.y = nIsHori ? std::max(fitScale.y, value) : value + fitScale.y;
        }
    }

    /* Adjust for border and padding of the parent */
    fitScale.x += nBorder.left + nBorder.right + nPadding.left + nPadding.right + nMarginLR;
    fitScale.y += nBorder.top + nBorder.bot + nPadding.top + nPadding.bot + nMarginTB;
    return fitScale;
}

auto BasicCalculator::calculateScaleForGenericElementOfTypeGrid(uinodes::UIBase* node,
    const glm::vec2 shrinkScaleBy) const -> void
{
    auto& nLayout = node->getBaseLayoutData();
    const auto& gridPolicy = nLayout.getGrid();
    if (gridPolicy.rows.empty() || gridPolicy.cols.empty()) { return; }

    /* Note for future: this precomputation doesn't need to be done each time, only when the grid policy changes. */
    calculatePrecomputedGridStartPos(node, shrinkScaleBy);

    const auto& nContentScale = nLayout.getContentBoxScale()- shrinkScaleBy;
    const uint32_t nRow = gridPolicy.rows.size();
    const uint32_t nCol = gridPolicy.cols.size();
    const auto& childNodes = node->getElements();
    for (auto& childNode : childNodes)
    {
        auto& chLayout = childNode->getBaseLayoutData();
        const auto& gridPos = chLayout.getGridPos();
        const auto& gridSpan = chLayout.getGridSpan();

        if (gridPos.col >= nCol) { continue; }
        if (gridPos.row >= nRow) { continue; }

        const glm::vec2 gridPosStart{
            gridPolicy.precompStart[gridPos.col],
            gridPolicy.precompStart[nCol + gridPos.row]
        };

        const glm::vec2 gridPosEnd{
            gridPos.col + gridSpan.col < nCol
                ? gridPolicy.precompStart[gridPos.col + gridSpan.col]
                : nContentScale.x,
            gridPos.row + gridSpan.row < nRow
                ? gridPolicy.precompStart[nCol + gridPos.row + gridSpan.row]
                : nContentScale.y,
        };

        chLayout.setComputedScale(gridPosEnd - gridPosStart);
    }
}

auto BasicCalculator::calculatePosForGenericElementOfTypeGrid(uinodes::UIBase* node,
    const glm::vec2 shrinkScaleBy) const -> void
{
    auto& gridPolicy = node->getBaseLayoutData().getGrid();
    const uint32_t nCol = gridPolicy.cols.size();
    const auto& childNodes = node->getElements();
    for (auto& childNode : childNodes)
    {
        auto& chLayout = childNode->getBaseLayoutData();
        const auto& gridPos = chLayout.getGridPos();
        chLayout.setComputedPos(
            {
                gridPolicy.precompStart[gridPos.col],
                gridPolicy.precompStart[nCol + gridPos.row]
            });
    }
}

auto BasicCalculator::calculatePrecomputedGridStartPos(uinodes::UIBase* node,
    const glm::vec2 shrinkScaleBy) const -> void
{
    /* Compute the amound of PX occupied space and FR parts in order to compute how much a FR part is worth. */
    auto& nLayout = node->getBaseLayoutData();
    auto& gridPolicy = nLayout.getGrid();
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

    const auto& pContentScale = nLayout.getContentBoxScale()- shrinkScaleBy;
    const float wFrac = (pContentScale.x - totalPx.x) / std::max(1.0f, totalFrac.x);
    const float hFrac = (pContentScale.y - totalPx.y) / std::max(1.0f, totalFrac.y);

    /* Fill the precomputed spots where each row/col starts. We only need to save the starts for one row and col.
    All the data is mashed in a flat array like this [cols_start.., rows_start..] */
    const uint32_t nRow = gridPolicy.rows.size();
    const uint32_t nCol = gridPolicy.cols.size();

    gridPolicy.precompStart.clear();
    gridPolicy.precompStart.reserve(nCol + nRow);

    glm::vec2 precompStart{nLayout.getContentBoxPos()};
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

// auto BasicCalculator::calculateSplitPaneHandlesScale(uinodes::UISplitPane* parent) const -> glm::vec2
// {
//     const auto& pLayoutType = parent->getType();
//     const auto& pContentScale = parent->getContentBoxScale();
//     const auto& elements = parent->getElements();

//     glm::vec2 handlesSize{0, 0};
//     for (const auto& element : elements)
//     {
//         if (element->getTypeId() != uinodes::UIButton::typeId) { continue; }

//         glm::vec2 cScale;
//         const auto& userScale = element->getScale();
//         if (pLayoutType == LayoutBase::Type::HORIZONTAL)
//         {
//             cScale.x = userScale.x.val;
//             cScale.y = pContentScale.y * userScale.y.val;
//         }
//         else if (pLayoutType == LayoutBase::Type::VERTICAL)
//         {
//             cScale.y = userScale.y.val;
//             cScale.x = pContentScale.x * userScale.x.val;
//         }

//         handlesSize += cScale;
//         element->setComputedScale(cScale);
//     }

//     return handlesSize;
// }

// auto BasicCalculator::calculateSplitPaneNonHandleScale(uinodes::UISplitPane* parent,
//     const glm::vec2 handlesSize) const -> void
// {
//     const auto& elements = parent->getElements();
//     const auto& pLayoutType = parent->getType();
//     const auto& pContentScale = parent->getContentBoxScale();
//     const auto& reducedPContentScale = pContentScale - handlesSize;
//     for (const auto& element : elements)
//     {
//         if (element->getTypeId() == uinodes::UIButton::typeId) { continue; }

//         const auto& userScale = element->getScale();
//         glm::vec2 cScale;

//         if (pLayoutType == LayoutBase::Type::HORIZONTAL)
//         {
//             cScale.x = reducedPContentScale.x * userScale.x.val;
//             cScale.y = pContentScale.y * userScale.y.val;
//             cScale.x = std::clamp(cScale.x, (float)element->getMinScale().x, (float)element->getMaxScale().x);
//         }
//         else if (pLayoutType == LayoutBase::Type::VERTICAL)
//         {
//             cScale.y = reducedPContentScale.y * userScale.y.val;
//             cScale.x = pContentScale.x * userScale.x.val;
//             cScale.y = std::clamp(cScale.y, (float)element->getMinScale().y, (float)element->getMaxScale().y);
//         }

//         element->setComputedScale(cScale);
//     }
// }

// auto BasicCalculator::applySplitPaneElementsScaleCorrection(uinodes::UISplitPane* parent,
//     const glm::vec2 handlesSize) const -> void
// {
//     const auto& elements = parent->getElements();
//     const auto& pContentScale = parent->getContentBoxScale();
//     const auto& reducedPContentScale = pContentScale - handlesSize;
//     for (int32_t handleIdx = 0; handleIdx < (int32_t)elements.size() - 1; ++handleIdx)
//     {
//         if (elements[handleIdx]->getTypeId() != uinodes::UIButton::typeId) { continue; }

//         glm::vec2 wantedOffsetRel{0, 0};

//         const uint32_t lPaneIdx = handleIdx - 1;
//         const uint32_t rPaneIdx = handleIdx + 1;
//         const glm::vec2 lpMinScaleRel = elements[lPaneIdx]->getMinScale() / reducedPContentScale;
//         const glm::vec2 lpMaxScaleRel = elements[lPaneIdx]->getMaxScale() / reducedPContentScale;
//         const glm::vec2 rpMinScaleRel = elements[rPaneIdx]->getMinScale() / reducedPContentScale;
//         const glm::vec2 rpMaxScaleRel = elements[rPaneIdx]->getMaxScale() / reducedPContentScale;

//         auto lScale = elements[lPaneIdx]->getScale();
//         auto rScale = elements[rPaneIdx]->getScale();
//         if (parent->getType() == LayoutBase::Type::HORIZONTAL)
//         {
//             if (lScale.x.val < lpMinScaleRel.x)
//             {
//                 wantedOffsetRel.x = lpMinScaleRel.x - lScale.x.val;
//             }

//             if (rScale.x.val < rpMinScaleRel.x)
//             {
//                 wantedOffsetRel.x = rScale.x.val - rpMinScaleRel.x;
//             }

//             if (lScale.x.val > lpMaxScaleRel.x)
//             {
//                 wantedOffsetRel.x = lpMaxScaleRel.x - lScale.x.val;
//             }

//             if (rScale.x.val > rpMaxScaleRel.x)
//             {
//                 wantedOffsetRel.x = rScale.x.val - rpMaxScaleRel.x;
//             }

//             /* Apply the relative offsets correction. */
//             lScale.x.val += wantedOffsetRel.x;
//             elements[lPaneIdx]->setScale(lScale);

//             rScale.x.val -= wantedOffsetRel.x;
//             elements[rPaneIdx]->setScale(rScale);

//             auto lCompScale = elements[lPaneIdx]->getComputedScale();
//             lCompScale.x = lScale.x.val * reducedPContentScale.x;
//             elements[lPaneIdx]->setComputedScale(lCompScale);

//             auto rCompScale = elements[rPaneIdx]->getComputedScale();
//             rCompScale.x = rScale.x.val * reducedPContentScale.x;
//             elements[rPaneIdx]->setComputedScale(rCompScale);
//         }
//         else if (parent->getType() == LayoutBase::Type::VERTICAL)
//         {
//             if (lScale.y.val < lpMinScaleRel.y)
//             {
//                 wantedOffsetRel.y = lpMinScaleRel.y - lScale.y.val;
//             }

//             if (rScale.y.val < rpMinScaleRel.y)
//             {
//                 wantedOffsetRel.y = rScale.y.val - rpMinScaleRel.y;
//             }

//             if (lScale.y.val > lpMaxScaleRel.y)
//             {
//                 wantedOffsetRel.y = lpMaxScaleRel.y - lScale.y.val;
//             }

//             if (rScale.y.val > rpMaxScaleRel.y)
//             {
//                 wantedOffsetRel.y = rScale.y.val - rpMaxScaleRel.y;
//             }

//             /* Apply the relative offsets correction. */
//             lScale.y.val += wantedOffsetRel.y;
//             elements[lPaneIdx]->setScale(lScale);

//             rScale.y.val -= wantedOffsetRel.y;
//             elements[rPaneIdx]->setScale(rScale);

//             auto lCompScale = elements[lPaneIdx]->getComputedScale();
//             lCompScale.y = lScale.y.val * reducedPContentScale.y;
//             elements[lPaneIdx]->setComputedScale(lCompScale);

//             auto rCompScale = elements[rPaneIdx]->getComputedScale();
//             rCompScale.y = rScale.y.val * reducedPContentScale.y;
//             elements[rPaneIdx]->setComputedScale(rCompScale);
//         }
//     }
// }

// auto BasicCalculator::calculateSplitPaneElementsPos(uinodes::UISplitPane* parent) const -> void
// {
//     const auto& elements = parent->getElements();
//     const auto& pLayoutType = parent->getType();
//     const auto& pContentPos = parent->getContentBoxPos();
//     glm::vec2 nextPos{pContentPos};
//     glm::vec2 pos{0, 0};
//     for (auto& element : elements)
//     {
//         const auto& margins = element->getMargin();
//         const auto& compScale = element->getComputedScale();
//         if (pLayoutType == LayoutBase::Type::HORIZONTAL)
//         {
//             pos = nextPos + glm::vec2{margins.left, margins.top};
//             nextPos.x = pos.x + compScale.x + margins.right;
//         }
//         else if (pLayoutType == LayoutBase::Type::VERTICAL)
//         {
//             pos = nextPos + glm::vec2{margins.left, margins.top};
//             nextPos.y = pos.y + compScale.y + margins.bot;
//         }

//         element->setComputedPos(pos);
//     }
// }

// auto BasicCalculator::calculateSplitPaneRelativeValuesDueToDrag(uinodes::UISplitPane* parent,
//     const uint32_t handleIdx, const glm::vec2 handlesSize, const glm::vec2 mousePos) const -> void
// {
//     const auto& elements = parent->getElements();
//     const auto& pContentBoxScale = parent->getContentBoxScale();
//     const auto& pLayoutType = parent->getType();

//     /* Calculate difference between the current mouse position and the handle's center. */
//     const auto handleCenter = elements[handleIdx]->getComputedPos()
//         + elements[handleIdx]->getComputedScale() * 0.5f;
//     const glm::vec2 mouseDiff = mousePos - handleCenter;

//     const uint32_t lPaneIdx = handleIdx - 1;
//     const uint32_t rPaneIdx = handleIdx + 1;

//     const auto& contentScale = pContentBoxScale - handlesSize;

//     glm::vec2 wantedOffsetRel = mouseDiff / contentScale;

//     const glm::vec2 lpMinScaleRel = elements[lPaneIdx]->getMinScale() / contentScale;
//     const glm::vec2 lpMaxScaleRel = elements[lPaneIdx]->getMaxScale() / contentScale;
//     const glm::vec2 rpMinScaleRel = elements[rPaneIdx]->getMinScale() / contentScale;
//     const glm::vec2 rpMaxScaleRel = elements[rPaneIdx]->getMaxScale() / contentScale;

//     /* Add the maximum amount of offset to the two panes without violating any constraints. */
//     auto lScale = elements[lPaneIdx]->getScale();
//     auto rScale = elements[rPaneIdx]->getScale();
//     if (pLayoutType == LayoutBase::Type::HORIZONTAL)
//     {
//         wantedOffsetRel.x = constrainOffset(wantedOffsetRel.x,
//             lScale.x.val, lpMinScaleRel.x, lpMaxScaleRel.x,
//             rScale.x.val, rpMinScaleRel.x, rpMaxScaleRel.x);

//         /* Apply the relative offsets. */
//         lScale.x.val += wantedOffsetRel.x;
//         elements[lPaneIdx]->setScale(lScale);

//         rScale.x.val -= wantedOffsetRel.x;
//         elements[rPaneIdx]->setScale(rScale);
//     }
//     else if (pLayoutType == LayoutBase::Type::VERTICAL)
//     {
//         wantedOffsetRel.y = constrainOffset(wantedOffsetRel.y,
//             lScale.y.val, lpMinScaleRel.y, lpMaxScaleRel.y,
//             rScale.y.val, rpMinScaleRel.y, rpMaxScaleRel.y);

//         /* Apply the relative offsets. */
//         lScale.y.val += wantedOffsetRel.y;
//         elements[lPaneIdx]->setScale(lScale);

//         rScale.y.val -= wantedOffsetRel.y;
//         elements[rPaneIdx]->setScale(rScale);
//     }
// }

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
