#pragma once

#include <LavenderUI/Node/UIBase.hpp>
#include <LavenderUI/Utils/Misc.hpp>
#include <LavenderUI/Node/InternalUse/UIScroll.hpp>

namespace lav::node
{
/**
    @brief
    Class represeting a pane which can hold other UI elements. Additionally it is also a scrollable
    area in case elements do overflow.
*/
class UIPane : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIPane, "elemVert.glsl", "elemFrag.glsl");

    auto setScrollEnabled(const bool enableHorizontal = true, const bool enableVertical = true) -> UIPane&;
    auto setScrollSensitivity(const float value) -> UIPane&;
    auto isVerticalOverflow() const -> bool;
    auto isHorizontalOverflow() const -> bool;
    auto getHorizontalScroll() const -> UIScrollWPtr;
    auto getVerticalScroll() const -> UIScrollWPtr;

protected:
    auto setInternalScrollOverflow(const glm::ivec2 newOverflow) -> bool;
    auto getClosestScrollbar(const glm::ivec2 pMouse) const -> uint32_t;

private:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(core::UIStatePtr& state) -> void override;

    auto calculateLayout() -> glm::ivec2;
    auto addAndSetIfNeeded(const UIScrollPtr scrollNode, const int32_t overflow) -> bool;
    auto removeAndSetIfNeeded(const UIScrollPtr scrollNode, const int32_t overflow) -> bool;

protected:
    UIScrollPtr hScroll_;
    UIScrollPtr vScroll_;
};
using UIPanePtr = std::shared_ptr<UIPane>;
using UIPaneWPtr = std::weak_ptr<UIPane>;
using UIPanePtrVec = std::vector<UIPanePtr>;
} // namespace lav::node
