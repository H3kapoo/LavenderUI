#include "BaseCalculator.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::core
{
auto BaseCalculator::get() -> BaseCalculator&
{
    static BaseCalculator instance;
    return instance;
}

BaseCalculator::BaseCalculator()
    : log_(__func__)
{}

auto BaseCalculator::calculateScaleForGenericElement(node::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> void
{
    const auto& elements = parent->getElements();
    if (elements.empty()) { return; }

    const auto& nLayout = parent->getBaseLayoutData();
    const auto& nType = nLayout.getType();
    if (nType == LayoutBase::Type::GRID)
    {
        return calculateScaleForGenericElementOfTypeGrid(parent, shrinkScaleBy);
    }

    /* Part of the parent area could be occupied by a scroll bar. We need to account for it. */
    const auto& nContentBoxScale = nLayout.getContentBoxScale() - shrinkScaleBy;

    glm::vec2 nonFillRunningTotal{0, 0};
    glm::vec2 fillsNeededPerAxis{0, 0};
    glm::ivec2 intTotal{0, 0};
    node::UIBasePtr lastElement{nullptr};

    for (const auto& element : elements)
    {
        SKIP_SCROLLBAR(element);

        glm::vec2 cScale{0, 0};
        auto& eLayout = element->getBaseLayoutData();
        const auto& userScale = eLayout.getScale();
        const auto& marginTB = eLayout.getTBMargin();
        const auto& marginLR = eLayout.getLRMargin();
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
        else if (isXFill && nLayout.isHorizontal()) { ++fillsNeededPerAxis.x; }

        if (isYPx) { cScale.y = userScale.y.val - marginTB; }
        else if (isYRel) { cScale.y = nContentBoxScale.y * userScale.y.val - marginTB; }
        else if (isYFill && nLayout.isVertical()) { ++fillsNeededPerAxis.y; }

        if (isXFit || isYFit)
        {
            /* Returns a mock `userScale` like value calculating what scale the element would be
                if it wrapped all it's children tightly. */
            const glm::vec2 fitScale = calculateFitScale(element.get());
            if (isXFit) { cScale.x = fitScale.x - marginLR; }
            if (isYFit) { cScale.y = fitScale.y - marginTB; }
        }

        nonFillRunningTotal += glm::ivec2{
            nLayout.isHorizontal() ? cScale.x : 0,
            nLayout.isVertical() ? cScale.y : 0
        };

        cScale = utils::round(cScale);
        eLayout.setComputedScale(cScale);

        intTotal += cScale;
        lastElement = element;
    }

    // if (intTotal.x < nonFillRunningTotal.x)
    // {
    //     log_.warn("total {}   intTotal {}", nonFillRunningTotal.x, intTotal.x);
    //     const float diff = nonFillRunningTotal.x - intTotal.x;
    //     auto& eLayout = lastElement->getBaseLayoutData();
    //     auto sc = eLayout.getComputedScale();
    //     sc.x -= diff;
    //     eLayout.setComputedScale(sc);
    //     log_.warn("newsc for last {}", sc.x);
    // }

    fillsNeededPerAxis = utils::max(fillsNeededPerAxis, {1, 1});

    /* Process the FILL nodes. */
    const glm::vec2 equalFillSpace = (nContentBoxScale - nonFillRunningTotal) / fillsNeededPerAxis;
    for (const auto& element : elements)
    {
        SKIP_SCROLLBAR(element);

        glm::vec2 cScale{0, 0};
        auto& eLayout = element->getBaseLayoutData();
        const auto& userScale = eLayout.getScale();
        const bool isXFill = userScale.x.type == LayoutBase::ScaleType::FILL;
        const bool isYFill = userScale.y.type == LayoutBase::ScaleType::FILL;

        if (!isXFill && !isYFill) { continue; }

        const auto& marginTB = eLayout.getTBMargin();
        const auto& marginLR = eLayout.getLRMargin();
        cScale = eLayout.getComputedScale();
        if (isXFill)
        {
            cScale.x = equalFillSpace.x - marginLR;
        }

        if (isYFill)
        {
            cScale.y = equalFillSpace.y - marginTB;
        }

        /* Note: Rounding errors again when using relative.. */
        cScale = utils::round(cScale);
        eLayout.setComputedScale(cScale);
    }
}

auto BaseCalculator::calculatePositionForGenericElement(node::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> void
{
    const auto& elements = parent->getElements();
    if (elements.empty()) { return; }

    const auto& pLayout = parent->getBaseLayoutData();
    if (pLayout.isGrid())
    {
        return calculatePosForGenericElementOfTypeGrid(parent, shrinkScaleBy);
    }

    const auto& pContentBoxPos = pLayout.getContentBoxPos();
    const auto& pContentBoxScale = pLayout.getContentBoxScale() - shrinkScaleBy;
    const auto& pContentBoxMaxPoint = pContentBoxPos + pContentBoxScale;
    const auto& pWrap = pLayout.getWrap();

    SpacingDetails spacingDetails = calculateSpacingOnAxis(parent, shrinkScaleBy);

    glm::vec2 nextPos{pContentBoxPos + spacingDetails.additionalStartPush};
    glm::vec2 computedPos{0, 0};
    glm::vec2 maxOnAxis{0, 0};
    for (auto& element : elements)
    {
        SKIP_SCROLLBAR(element);

        auto& eLayout = element->getBaseLayoutData();
        const auto& margins = eLayout.getMargin();
        const auto& userPos = eLayout.getPos();
        if (userPos.x.type == LayoutBase::PositionType::ABS || userPos.y.type == LayoutBase::PositionType::ABS)
        {
            // TODO: Take into consideration element's margins.
            // Margins will push the computed position inwards.
            eLayout.setComputedPos({userPos.x.val, userPos.y.val});
            continue;
        }

        const auto& compScale = eLayout.getComputedScale();
        const glm::vec2 fullBoxScale = eLayout.getFullBoxScale();
        const glm::vec2 nextMaxPoint = nextPos + fullBoxScale;
        const glm::vec2 marginPush = glm::vec2{margins.left, margins.top};

        /* Note: `nextPos` starts at the end of the previous' element margin end. */
        if (pLayout.isHorizontal())
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
        else if (pLayout.isVertical())
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

        eLayout.setComputedPos(computedPos);
    }
}

auto BaseCalculator::calculateAlignmentForElements(node::UIBase* node,
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
        SKIP_SCROLLBAR(childNode);
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

auto BaseCalculator::calculateElementOverflow(node::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> glm::vec2
{
    glm::vec2 boxScale{0, 0};
    const auto& elements = parent->getElements();
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pContentPos = pLayout.getContentBoxPos();
    const auto& pContentScale = pLayout.getContentBoxScale() - shrinkScaleBy;
    for (const auto& element : elements)
    {
        /* Shall not be taken into consideration for overflow */
        SKIP_SCROLLBAR(element);

        const auto& eLayout = element->getBaseLayoutData();
        const auto& fullPos = eLayout.getFullBoxPos();
        const auto& fullScale = eLayout.getFullBoxScale();
        boxScale.x = std::max(boxScale.x, fullPos.x + fullScale.x);
        boxScale.y = std::max(boxScale.y, fullPos.y + fullScale.y);
    }

    return boxScale - (pContentPos + pContentScale);
}

auto BaseCalculator::solveRelativeScaling(node::UIBase* parent, const glm::ivec2 availableScale,
    const glm::ivec2 currentTotal, const bool skipButtonType) const -> void
{
#define OPERATION(operation, onWhat, decInc)                                                    \
    if (int32_t diff = operation; diff > 0)                                                     \
    {                                                                                           \
        for (const auto& element : elements)                                                    \
        {                                                                                       \
            if (diff <= 0) { return; }                                                          \
            if (skipButtonType && element->getTypeId() == node::UIButton::typeId) { continue; } \
            auto& eLayout = element->getBaseLayoutData();                                       \
            auto cScale = eLayout.getComputedScale();                                           \
            onWhat += decInc;                                                                   \
            diff -= 1;                                                                          \
            eLayout.setComputedScale(cScale);                                                   \
        }                                                                                       \
    }                                                                                           \

    /* If total objects scale is less than what the parent can allow, scale elements up by error */
    /* If total objects scale is greater than what the parent can allow, scale elements down by error */
    const auto& elements = parent->getElements();
    if (parent->getBaseLayoutData().isHorizontal())
    {
        OPERATION(availableScale.x - currentTotal.x, cScale.x, +1);
        OPERATION(currentTotal.x - availableScale.x, cScale.x, -1);
    }
    else if (parent->getBaseLayoutData().isVertical())
    {
        OPERATION(availableScale.y - currentTotal.y, cScale.y, +1);
        OPERATION(currentTotal.y - availableScale.y, cScale.y, -1);
    }
#undef OPERATION
}

auto BaseCalculator::calculateSpacingOnAxis(node::UIBase* parent,
    const glm::vec2 shrinkScaleBy) const -> SpacingDetails
{
    const auto& elements = parent->getElements();
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pSpacing = pLayout.getSpacing();
    const auto& nContentBoxScale = pLayout.getContentBoxScale();

    glm::vec2 computedSpacing{0, 0};
    glm::vec2 additionalStartPush{0,};

    /* Calculate max running scale and valid elements for spacing calculations. */
    glm::vec2 maxRunningScale{shrinkScaleBy};
    int32_t elementCountForSpacing{0};
    for (const auto& element : elements)
    {
        SKIP_SCROLLBAR(element);
        maxRunningScale += element->getBaseLayoutData().getFullBoxScale();
        ++elementCountForSpacing;
    }

    glm::vec2 nextPos{pLayout.getContentBoxPos()};
    glm::vec2 computedPos{0, 0};
    glm::vec2 maxOnAxis{0, 0};

    switch (pSpacing)
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

    if (pLayout.isHorizontal())
    {
        computedSpacing.y = 0;
        additionalStartPush.y = 0;
        return SpacingDetails{additionalStartPush, computedSpacing};
    }
    else if (pLayout.isVertical())
    {
        computedSpacing.x = 0;
        additionalStartPush.x = 0;
        return SpacingDetails{additionalStartPush, computedSpacing};
    }

    return SpacingDetails{};
}

auto BaseCalculator::calculateFitScale(node::UIBase* parent) const -> glm::vec2
{
    const auto& elements = parent->getElements();
    if (elements.empty())
    {
        log_.warn("no elements for fit");
        return {0, 0};
    }

    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pMarginLR = pLayout.getLRMargin();
    const auto& pMarginTB = pLayout.getTBMargin();
    const auto& pBorder = pLayout.getBorder();
    const auto& pPadding = pLayout.getPadding();
    const auto& pUserScale = pLayout.getScale();
    const bool nIsXFit = pUserScale.x.type == LayoutBase::ScaleType::FIT;
    const bool nIsYFit = pUserScale.y.type == LayoutBase::ScaleType::FIT;

    glm::vec2 fitScale{0, 0};
    for (const auto& element : elements)
    {
        SKIP_SCROLLBAR(element);
        const auto& userScale = element->getBaseLayoutData().getScale();
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
        if (nIsXFit)
        {
            if (isXPx) { value = userScale.x.val; }
            else if (isXFit) { value = internalFitScale.x; }
            fitScale.x = pLayout.isVertical() ? std::max(fitScale.x, value) : value + fitScale.x;
        }

        if (nIsYFit)
        {
            if (isYPx) { value = userScale.y.val; }
            else if (isYFit) { value = internalFitScale.y; }
            fitScale.y = pLayout.isHorizontal() ? std::max(fitScale.y, value) : value + fitScale.y;
        }
    }

    /* Adjust for border and padding of the parent */
    fitScale.x += pBorder.left + pBorder.right + pPadding.left + pPadding.right + pMarginLR;
    fitScale.y += pBorder.top + pBorder.bot + pPadding.top + pPadding.bot + pMarginTB;
    return fitScale;
}

auto BaseCalculator::calculateScaleForGenericElementOfTypeGrid(node::UIBase* node,
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

auto BaseCalculator::calculatePosForGenericElementOfTypeGrid(node::UIBase* node,
    const glm::vec2 shrinkScaleBy) const -> void
{
    (void)shrinkScaleBy;
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

auto BaseCalculator::calculatePrecomputedGridStartPos(node::UIBase* node,
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
} // namespace lav::core
