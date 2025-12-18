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
    const glm::ivec2 handlesSize = calculateHandlesScale(parent);

    calculateNonHandlElementsScale(parent, handlesSize);

    calculateElementsScaleCorrection(parent, handlesSize);

    calculateElementsPos(parent);

    calculateRelativeValuesDueToDrag(parent, handleIdx, handlesSize, mousePos);
}

auto SplitPaneCalculator::calculateHandlesScale(node::UISplitPane* parent) const -> glm::ivec2
{
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pContentScale = pLayout.getContentBoxScale();
    const auto& elements = parent->getElements();

    glm::ivec2 handlesSize{0, 0};
    for (const auto& element : elements)
    {
        if (element->getTypeId() != node::UIButton::typeId) { continue; }

        auto& eLayout = element->getBaseLayoutData();
        const auto& userScale = eLayout.getScale();

        glm::vec2 cScale;
        if (pLayout.isHorizontal())
        {
            cScale.x = userScale.x.val;
            cScale.y = pContentScale.y * userScale.y.val;
            handlesSize.x += cScale.x;
        }
        else if (pLayout.isVertical())
        {
            cScale.y = userScale.y.val;
            cScale.x = pContentScale.x * userScale.x.val;
            handlesSize.y += cScale.y;
        }

        eLayout.setComputedScale(cScale);
    }

    return handlesSize;
}

auto SplitPaneCalculator::calculateNonHandlElementsScale(node::UISplitPane* parent,
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

auto SplitPaneCalculator::calculateElementsScaleCorrection(node::UISplitPane* parent,
    const glm::vec2 handlesSize) const -> void
{
#define OPERATION_WANTED_OFFSET_ON_AXIS(axis)                                                               \
    /*                                                                                                      \
        Current left pane relative scale is less than left pane min scale which means we need to increase   \
        the left pane relative scale by the difference it would take to achieve that minimum.               \
        So take the min scale minus the current relative scale as the wanted offset.                        \
    */                                                                                                      \
    if (lUserScale. axis .val < lpMinScaleRel. axis)                                                        \
    {                                                                                                       \
        wantedOffsetRel. axis = lpMinScaleRel. axis - lUserScale. axis .val;                                \
    }                                                                                                       \
                                                                                                            \
    /* Exactly the same as above just that now we be looking at the right pane. */                          \
    if (rUserScale. axis .val < rpMinScaleRel. axis)                                                        \
    {                                                                                                       \
        wantedOffsetRel. axis = rUserScale. axis .val - rpMinScaleRel. axis;                                \
    }                                                                                                       \
                                                                                                            \
    /*                                                                                                      \
        Current left pane relative scale is greater than left pane max scale which means we need to decrease\
        the left pane relative scale by the difference it would take to achieve that maximum.               \
        So take the max scale minus the current relative scale as the wanted offset.                        \
    */                                                                                                      \
    if (lUserScale. axis .val > lpMaxScaleRel. axis)                                                        \
    {                                                                                                       \
        wantedOffsetRel. axis = lpMaxScaleRel. axis - lUserScale. axis .val;                                \
    }                                                                                                       \
                                                                                                            \
    /* Exactly the same as above just that now we be looking at the right pane. */                          \
    if (rUserScale. axis .val > rpMaxScaleRel. axis)                                                        \
    {                                                                                                       \
        wantedOffsetRel. axis = rUserScale. axis .val - rpMaxScaleRel. axis;                                \
    }                                                                                                       \
                                                                                                            \
    /* Apply the relative offsets correction. */                                                            \
    lUserScale. axis .val += wantedOffsetRel. axis;                                                         \
    lpLayout.setScale(lUserScale);                                                                          \
                                                                                                            \
    rUserScale. axis .val -= wantedOffsetRel. axis;                                                         \
    rpLayout.setScale(rUserScale);                                                                          \


    const auto& elements = parent->getElements();
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pContentScale = pLayout.getContentBoxScale();
    const auto& reducedPContentScale = pContentScale - handlesSize;
    glm::ivec2 runningTotal{0, 0};
    for (int32_t handleIdx = 0; handleIdx < (int32_t)elements.size() - 1; ++handleIdx)
    {
        if (elements[handleIdx]->getTypeId() != node::UIButton::typeId) { continue; }

        auto& lpLayout = elements[handleIdx - 1]->getBaseLayoutData();
        auto& rpLayout = elements[handleIdx + 1]->getBaseLayoutData();
        auto lUserScale = lpLayout.getScale();
        auto rUserScale = rpLayout.getScale();
        const glm::vec2 lpMinScaleRel = lpLayout.getMinScale() / reducedPContentScale;
        const glm::vec2 lpMaxScaleRel = lpLayout.getMaxScale() / reducedPContentScale;
        const glm::vec2 rpMinScaleRel = rpLayout.getMinScale() / reducedPContentScale;
        const glm::vec2 rpMaxScaleRel = rpLayout.getMaxScale() / reducedPContentScale;

        glm::vec2 wantedOffsetRel{0, 0};

        if (pLayout.isHorizontal()) { OPERATION_WANTED_OFFSET_ON_AXIS(x); }
        else if (pLayout.isVertical()) { OPERATION_WANTED_OFFSET_ON_AXIS(y); }
        else { log_.error("Unsupported orientation for: {}", __func__); }
    }

#undef OPERATION
}

auto SplitPaneCalculator::calculateElementsPos(node::UISplitPane* parent) const -> void
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

auto SplitPaneCalculator::calculateRelativeValuesDueToDrag(node::UISplitPane* parent,
    const uint32_t handleIdx, const glm::vec2 handlesSize, const glm::vec2 mousePos) const -> void
{
#define OPERATION_KEEP_CONSTRAINTS_ON_AXIS(axis)                        \
    wantedOffsetRel. axis  = constrainOffset(wantedOffsetRel. axis ,    \
        lScale. axis .val, lpMinScaleRel. axis , lpMaxScaleRel. axis ,  \
        rScale. axis .val, rpMinScaleRel. axis , rpMaxScaleRel. axis ); \
                                                                        \
    /* Apply the relative offsets. */                                   \
    lScale. axis .val += wantedOffsetRel. axis ;                        \
    lLayout.setScale(lScale);                                           \
                                                                        \
    rScale. axis .val -= wantedOffsetRel. axis ;                        \
    rLayout.setScale(rScale);                                           \


    /* Do nothing if we have no handle active. */
    if (!handleIdx) { return; }

    const auto& elements = parent->getElements();
    const auto& pLayout = parent->getBaseLayoutData();
    const auto& pContentBoxScale = pLayout.getContentBoxScale();

    /*
        Calculate difference between the current mouse position and the handle's center. Do the calculations
        as if the user always drags from the center of the handle.
    */
    const auto& contentScale = pContentBoxScale - handlesSize;
    const auto& handleLayout = elements[handleIdx]->getBaseLayoutData();
    const auto handleCenter = handleLayout.getComputedPos() + handleLayout.getComputedScale() * 0.5f;
    const glm::vec2 mouseDiff = mousePos - handleCenter;

    auto& lLayout = elements[handleIdx - 1]->getBaseLayoutData();
    auto& rLayout = elements[handleIdx + 1]->getBaseLayoutData();
    const glm::vec2 lpMinScaleRel = lLayout.getMinScale() / contentScale;
    const glm::vec2 lpMaxScaleRel = lLayout.getMaxScale() / contentScale;
    const glm::vec2 rpMinScaleRel = rLayout.getMinScale() / contentScale;
    const glm::vec2 rpMaxScaleRel = rLayout.getMaxScale() / contentScale;

    /* Wanted relative scale at which we want to "find" the handle at the calculations end. */
    glm::vec2 wantedOffsetRel = mouseDiff / contentScale;

    /* Add the maximum amount of offset to the two panes without violating any constraints. */
    auto lScale = lLayout.getScale();
    auto rScale = rLayout.getScale();
    if (pLayout.isHorizontal()) { OPERATION_KEEP_CONSTRAINTS_ON_AXIS(x); }
    else if (pLayout.isVertical()) { OPERATION_KEEP_CONSTRAINTS_ON_AXIS(y); }

#undef OPERATION_KEEP_CONSTRAINTS_ON_AXIS
}

auto SplitPaneCalculator::constrainOffset(float wantedOffset,
    const float lpScale, const float lpMin, const float lpMax,
    const float rpScale, const float rpMin, const float rpMax) const -> float
{
    /* If left pane will go under min scale by adding the wantedOffset then cap the wanted
        offset to the maximum possible value to add. */
    if (lpScale + wantedOffset < lpMin)
    {
        wantedOffset = lpMin - lpScale;
    }

    /* If right pane will go under min scale by subtracting the wantedOffset then cap the wanted
        offset to the maximum possible value to subtract. */
    if (rpScale - wantedOffset < rpMin)
    {
        wantedOffset = rpScale - rpMin;
    }

    /* If left pane will go above max scale by adding the wantedOffset then cap the wanted
        offset to the maximum possible value to add. */
    if (lpScale + wantedOffset > lpMax)
    {
        wantedOffset = lpMax - lpScale;
    }

    /* If right pane will go above max scale by subtracting the wantedOffset then cap the wanted
        offset to the maximum possible value to subtract. */
    if (rpScale - wantedOffset > rpMax)
    {
        wantedOffset = rpScale - rpMax;
    }

    return wantedOffset;
}
} // namespace lav::core