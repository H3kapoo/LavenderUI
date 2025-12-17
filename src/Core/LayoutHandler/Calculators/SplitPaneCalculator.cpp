#include "SplitPaneCalculator.hpp"

namespace lav::core
{
auto SplitPaneCalculator::get() -> SplitPaneCalculator&
{
    static SplitPaneCalculator instance;
    return instance;
}

SplitPaneCalculator::SplitPaneCalculator()
    : log_(__func__)
{}

auto SplitPaneCalculator::calculateSplitPaneElements(node::UISplitPane* parent, const uint32_t handleIdx,
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

auto SplitPaneCalculator::calculateSplitPaneHandlesScale(node::UISplitPane* parent) const -> glm::vec2
{
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pContentScale = pLayout.getContentBoxScale();
    const auto& elements = parent->getElements();

    glm::vec2 handlesSize{0, 0};
    for (const auto& element : elements)
    {
        if (element->getTypeId() != node::UIButton::typeId) { continue; }

        auto& eLayout = element->getBaseLayoutData();
        glm::vec2 cScale;
        const auto& userScale = eLayout.getScale();

        if (pLayout.isHorizontal())
        {
            cScale.x = userScale.x.val;
            cScale.y = pContentScale.y * userScale.y.val;
        }
        else if (pLayout.isVertical())
        {
            cScale.y = userScale.y.val;
            cScale.x = pContentScale.x * userScale.x.val;
        }

        handlesSize += cScale;
        eLayout.setComputedScale(cScale);
    }

    return handlesSize;
}

auto SplitPaneCalculator::calculateSplitPaneNonHandleScale(node::UISplitPane* parent,
    const glm::vec2 handlesSize) const -> void
{
    const auto& elements = parent->getElements();
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pContentScale = pLayout.getContentBoxScale();
    const auto& reducedPContentScale = pContentScale - handlesSize;
    glm::ivec2 runningTotal{0, 0};
    for (const auto& element : elements)
    {
        if (element->getTypeId() == node::UIButton::typeId) { continue; }

        auto& eLayout = element->getBaseLayoutData();
        const auto& userScale = eLayout.getScale();
        glm::vec2 cScale;

        if (pLayout.isHorizontal())
        {
            cScale.x = reducedPContentScale.x * userScale.x.val;
            cScale.x = std::clamp(cScale.x, (float)eLayout.getMinScale().x, (float)eLayout.getMaxScale().x);
            cScale.y = pContentScale.y * userScale.y.val;

            cScale.x = std::round(cScale.x);
            runningTotal.x += cScale.x;
        }
        else if (pLayout.isVertical())
        {
            cScale.y = reducedPContentScale.y * userScale.y.val;
            cScale.x = pContentScale.x * userScale.x.val;
            cScale.y = std::clamp(cScale.y, (float)eLayout.getMinScale().y, (float)eLayout.getMaxScale().y);

            cScale.y = std::round(cScale.y);
            runningTotal.y += cScale.y;
        }

        eLayout.setComputedScale(cScale);
    }

    solveRelativeScaling(parent, reducedPContentScale, runningTotal);
}

auto SplitPaneCalculator::applySplitPaneElementsScaleCorrection(node::UISplitPane* parent,
    const glm::vec2 handlesSize) const -> void
{
    const auto& elements = parent->getElements();
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pContentScale = pLayout.getContentBoxScale();
    const auto& reducedPContentScale = pContentScale - handlesSize;
    glm::ivec2 runningTotal{0, 0};
    for (int32_t handleIdx = 0; handleIdx < (int32_t)elements.size() - 1; ++handleIdx)
    {
        if (elements[handleIdx]->getTypeId() != node::UIButton::typeId) { continue; }

        glm::vec2 wantedOffsetRel{0, 0};

        const uint32_t lPaneIdx = handleIdx - 1;
        const uint32_t rPaneIdx = handleIdx + 1;
        auto& lLayout = elements[lPaneIdx]->getBaseLayoutData();
        auto& rLayout = elements[rPaneIdx]->getBaseLayoutData();
        const glm::vec2 lpMinScaleRel = lLayout.getMinScale() / reducedPContentScale;
        const glm::vec2 lpMaxScaleRel = lLayout.getMaxScale() / reducedPContentScale;
        const glm::vec2 rpMinScaleRel = rLayout.getMinScale() / reducedPContentScale;
        const glm::vec2 rpMaxScaleRel = rLayout.getMaxScale() / reducedPContentScale;

        auto lScale = lLayout.getScale();
        auto rScale = rLayout.getScale();
        if (pLayout.isHorizontal())
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
            lLayout.setScale(lScale);

            rScale.x.val -= wantedOffsetRel.x;
            rLayout.setScale(rScale);
        }
        else if (pLayout.isVertical())
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
            lLayout.setScale(lScale);

            rScale.y.val -= wantedOffsetRel.y;
            rLayout.setScale(rScale);
        }
    }
}

auto SplitPaneCalculator::calculateSplitPaneElementsPos(node::UISplitPane* parent) const -> void
{
    const auto& elements = parent->getElements();
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pContentPos = pLayout.getContentBoxPos();
    glm::vec2 nextPos{pContentPos};
    glm::vec2 pos{0, 0};
    for (auto& element : elements)
    {
        auto& eLayout = element->getBaseLayoutData();
        const auto& margins = eLayout.getMargin();
        const auto& compScale = eLayout.getComputedScale();
        if (pLayout.isHorizontal())
        {
            pos = nextPos + glm::vec2{margins.left, margins.top};
            nextPos.x = pos.x + compScale.x + margins.right;
        }
        else if (pLayout.isVertical())
        {
            pos = nextPos + glm::vec2{margins.left, margins.top};
            nextPos.y = pos.y + compScale.y + margins.bot;
        }

        eLayout.setComputedPos(pos);
    }
}

auto SplitPaneCalculator::calculateSplitPaneRelativeValuesDueToDrag(node::UISplitPane* parent,
    const uint32_t handleIdx, const glm::vec2 handlesSize, const glm::vec2 mousePos) const -> void
{
    const auto& elements = parent->getElements();
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pContentBoxScale = pLayout.getContentBoxScale();

    /* Calculate difference between the current mouse position and the handle's center. */
    const auto& handleLayout = elements[handleIdx]->getBaseLayoutData();
    const auto handleCenter = handleLayout.getComputedPos() + handleLayout.getComputedScale() * 0.5f;
    const glm::vec2 mouseDiff = mousePos - handleCenter;

    const uint32_t lPaneIdx = handleIdx - 1;
    const uint32_t rPaneIdx = handleIdx + 1;

    const auto& contentScale = pContentBoxScale - handlesSize;

    glm::vec2 wantedOffsetRel = mouseDiff / contentScale;

    auto& lLayout = elements[lPaneIdx]->getBaseLayoutData();
    auto& rLayout = elements[rPaneIdx]->getBaseLayoutData();
    const glm::vec2 lpMinScaleRel = lLayout.getMinScale() / contentScale;
    const glm::vec2 lpMaxScaleRel = lLayout.getMaxScale() / contentScale;
    const glm::vec2 rpMinScaleRel = rLayout.getMinScale() / contentScale;
    const glm::vec2 rpMaxScaleRel = rLayout.getMaxScale() / contentScale;

    /* Add the maximum amount of offset to the two panes without violating any constraints. */
    auto lScale = lLayout.getScale();
    auto rScale = rLayout.getScale();
    if (pLayout.isHorizontal())
    {
        wantedOffsetRel.x = constrainOffset(wantedOffsetRel.x,
            lScale.x.val, lpMinScaleRel.x, lpMaxScaleRel.x,
            rScale.x.val, rpMinScaleRel.x, rpMaxScaleRel.x);

        /* Apply the relative offsets. */
        lScale.x.val += wantedOffsetRel.x;
        lLayout.setScale(lScale);

        rScale.x.val -= wantedOffsetRel.x;
        rLayout.setScale(rScale);
    }
    else if (pLayout.isVertical())
    {
        wantedOffsetRel.y = constrainOffset(wantedOffsetRel.y,
            lScale.y.val, lpMinScaleRel.y, lpMaxScaleRel.y,
            rScale.y.val, rpMinScaleRel.y, rpMaxScaleRel.y);

        /* Apply the relative offsets. */
        lScale.y.val += wantedOffsetRel.y;
        lLayout.setScale(lScale);

        rScale.y.val -= wantedOffsetRel.y;
        rLayout.setScale(rScale);
    }
}

auto SplitPaneCalculator::constrainOffset(float wantedOffset,
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
} // namespace lav::core