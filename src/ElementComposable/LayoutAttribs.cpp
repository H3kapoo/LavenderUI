#include "LayoutAttribs.hpp"

#include "vendor/glm/gtc/matrix_transform.hpp"
#include "vendor/glm/trigonometric.hpp"

namespace src::elementcomposable
{
auto LayoutAttribs::isPointInside(const glm::ivec2& p) const -> bool
{
    return (p.x >= computedPos_.x && p.x <= computedPos_.x + computedScale_.x)
        && (p.y >= computedPos_.y && p.y <= computedPos_.y + computedScale_.y);
}

auto LayoutAttribs::isPointInsideView(const glm::ivec2& p) const -> bool
{
    return (p.x >= viewPos_.x && p.x <= viewPos_.x + viewScale_.x)
        && (p.y >= viewPos_.y && p.y <= viewPos_.y + viewScale_.y);
}

auto LayoutAttribs::computeViewBox(const LayoutAttribs& parentAttribs) -> void
{
    /* Take the intersection between this object's pos+scale & parent's already computed
        viewable area box.*/
    viewPos_ = {
        std::max(parentAttribs.viewPos_.x, computedPos_.x),
        std::max(parentAttribs.viewPos_.y, computedPos_.y)
    };

    const glm::vec2 pComposed = parentAttribs.viewPos_ + parentAttribs.viewScale_;
    const glm::vec2 thisComposed = computedPos_ + computedScale_;
    viewScale_ = {
        std::max(0.0f, std::min(pComposed.x, thisComposed.x) - viewPos_.x),
        std::max(0.0f, std::min(pComposed.y, thisComposed.y) - viewPos_.y)
    };
}

auto LayoutAttribs::getLayoutTransform() -> const glm::mat4&
{
    transform_ = glm::mat4{1.0f};
    transform_ = glm::translate(transform_, glm::vec3(computedPos_, index_));
    // transform_ = glm::rotate(transform_, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    transform_ = glm::scale(transform_, glm::vec3(computedScale_, 1.0f));
    return transform_;
}

auto LayoutAttribs::getLRMargin() const -> int32_t
{
    return margin.left + margin.right;
}

auto LayoutAttribs::getTBMargin() const -> int32_t
{
    return margin.top + margin.bot;
}

auto LayoutAttribs::getFullBoxPos() const -> glm::vec2
{
    return {computedPos_.x - margin.left, computedPos_.y - margin.top};
}

auto LayoutAttribs::getFullBoxScale() const -> glm::vec2
{
    return {computedScale_.x + margin.left + margin.right, computedScale_.y + margin.top + margin.bot};
}

auto LayoutAttribs::getContentBoxPos() const -> glm::vec2
{
    return {computedPos_.x + padding.left + border.left, computedPos_.y + padding.top + border.top};
}

auto LayoutAttribs::getContentBoxScale() const -> glm::vec2
{
    return {computedScale_.x - padding.left - padding.right - border.left - border.right,
        computedScale_.y - padding.top - padding.bot - border.top - border.bot};
}

auto LayoutAttribs::getLayoutType() const -> Type { return layoutType_; }
auto LayoutAttribs::getLayoutMargin() const -> const TBLR& { return margin; }
auto LayoutAttribs::getLayoutPadding() const -> const TBLR& { return padding; }
auto LayoutAttribs::getLayoutBorder() const -> const TBLR& { return border; }
auto LayoutAttribs::getLayoutBorderRadius() const -> const TBLR& { return borderRadius; }
auto LayoutAttribs::getLayoutShadow() const -> const TBLR& { return shadow; }
auto LayoutAttribs::getLayoutWrap() const -> bool { return wrap; }
auto LayoutAttribs::getLayoutPos() const -> const PositionXY& { return userPos_; }
auto LayoutAttribs::getLayoutScale() const -> const ScaleXY& { return userScale_; }
auto LayoutAttribs::getLayoutComputedPos() const -> const glm::vec2& { return computedPos_; }
auto LayoutAttribs::getLayoutComputedScale() const -> const glm::vec2& { return computedScale_; }
auto LayoutAttribs::getLayoutViewPos() const -> const glm::vec2& { return viewPos_; }
auto LayoutAttribs::getLayoutViewScale() const -> const glm::vec2& { return viewScale_; }
auto LayoutAttribs::getLayoutIndex() const -> uint32_t { return index_; }
auto LayoutAttribs::getLayoutAngle() const -> float { return angle_; }

auto LayoutAttribs::setLayoutType(Type val) -> LayoutAttribs& { layoutType_ = val; return *this; }
auto LayoutAttribs::setLayoutMargin(const TBLR& val) -> LayoutAttribs& { margin = val; return *this; }
auto LayoutAttribs::setLayoutPadding(const TBLR& val) -> LayoutAttribs& { padding = val; return *this; }
auto LayoutAttribs::setLayoutBorder(const TBLR& val) -> LayoutAttribs& { border = val; return *this; }
auto LayoutAttribs::setLayoutBorderRadius(const TBLR& val) -> LayoutAttribs& { borderRadius = val; return *this;}
auto LayoutAttribs::setLayoutShadow(const TBLR& val) -> LayoutAttribs& { shadow = val; return *this; }
auto LayoutAttribs::setLayoutWrap(const bool val) -> LayoutAttribs& { wrap = val; return *this; }
auto LayoutAttribs::setLayoutPos(const PositionXY& val) -> LayoutAttribs& { userPos_ = val; return *this; }
auto LayoutAttribs::setLayoutScale(const ScaleXY& val) -> LayoutAttribs& { userScale_ = val; return *this; }
auto LayoutAttribs::setLayoutComputedPos(const glm::vec2& val) -> LayoutAttribs& { computedPos_ = val; return *this; }
auto LayoutAttribs::setLayoutComputedScale(const glm::vec2& val) -> LayoutAttribs& {computedScale_ = val;return *this;}
auto LayoutAttribs::setLayoutViewPos(const glm::vec2& val) -> LayoutAttribs& { viewPos_ = val; return *this; }
auto LayoutAttribs::setLayoutViewScale(const glm::vec2& val) -> LayoutAttribs& { viewScale_ = val; return *this; }
auto LayoutAttribs::setLayoutIndex(uint32_t val) -> LayoutAttribs& { index_ = val;  return *this; }
auto LayoutAttribs::setLayoutEnableCustomIndex(const bool val) -> LayoutAttribs& { isCustomIndex_ = val;  return *this; }
auto LayoutAttribs::setLayoutAngle(float val) -> LayoutAttribs& { angle_ = val; return *this; }

LayoutAttribs::Scale operator"" _px(unsigned long long value)
{
    /* Loss of precision justified. */
    return {.val = (float)value, .type = LayoutAttribs::ScaleType::PX};
}

LayoutAttribs::Scale operator"" _rel(long double value)
{
    return {.val = (float)value, .type = LayoutAttribs::ScaleType::REL};
}
} // namespace src::elementcomposable