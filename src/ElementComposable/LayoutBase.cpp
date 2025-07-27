#include "LayoutBase.hpp"

#include "vendor/glm/gtc/matrix_transform.hpp"
#include "vendor/glm/trigonometric.hpp"

namespace src::elementcomposable
{
auto LayoutBase::isPointInside(const glm::ivec2& p) const -> bool
{
    return (p.x >= computedPos_.x && p.x <= computedPos_.x + computedScale_.x)
        && (p.y >= computedPos_.y && p.y <= computedPos_.y + computedScale_.y);
}

auto LayoutBase::isPointInsideView(const glm::ivec2& p) const -> bool
{
    return (p.x >= viewPos_.x && p.x <= viewPos_.x + viewScale_.x)
        && (p.y >= viewPos_.y && p.y <= viewPos_.y + viewScale_.y);
}

auto LayoutBase::computeViewBox(const LayoutBase& parentAttribs) -> void
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

auto LayoutBase::getTransform() -> const glm::mat4&
{
    transform_ = glm::mat4{1.0f};
    transform_ = glm::translate(transform_, glm::vec3(computedPos_, index_));
    // transform_ = glm::rotate(transform_, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    transform_ = glm::scale(transform_, glm::vec3(computedScale_, 1.0f));
    return transform_;
}

auto LayoutBase::getLRMargin() const -> int32_t
{
    return margin.left + margin.right;
}

auto LayoutBase::getTBMargin() const -> int32_t
{
    return margin.top + margin.bot;
}

auto LayoutBase::getFullBoxPos() const -> glm::vec2
{
    return {computedPos_.x - margin.left, computedPos_.y - margin.top};
}

auto LayoutBase::getFullBoxScale() const -> glm::vec2
{
    return {computedScale_.x + margin.left + margin.right, computedScale_.y + margin.top + margin.bot};
}

auto LayoutBase::getContentBoxPos() const -> glm::vec2
{
    return {computedPos_.x + padding.left + border.left, computedPos_.y + padding.top + border.top};
}

auto LayoutBase::getContentBoxScale() const -> glm::vec2
{
    return {computedScale_.x - padding.left - padding.right - border.left - border.right,
        computedScale_.y - padding.top - padding.bot - border.top - border.bot};
}

auto LayoutBase::getType() const -> Type { return layoutType_; }
auto LayoutBase::getMargin() const -> const TBLR& { return margin; }
auto LayoutBase::getPadding() const -> const TBLR& { return padding; }
auto LayoutBase::getBorder() const -> const TBLR& { return border; }
auto LayoutBase::getBorderRadius() const -> const TBLR& { return borderRadius; }
auto LayoutBase::getShadow() const -> const TBLR& { return shadow; }
auto LayoutBase::getWrap() const -> bool { return wrap; }
auto LayoutBase::getPos() const -> const PositionXY& { return userPos_; }
auto LayoutBase::getScale() const -> const ScaleXY& { return userScale_; }
auto LayoutBase::getComputedPos() const -> const glm::vec2& { return computedPos_; }
auto LayoutBase::getComputedScale() const -> const glm::vec2& { return computedScale_; }
auto LayoutBase::getViewPos() const -> const glm::vec2& { return viewPos_; }
auto LayoutBase::getViewScale() const -> const glm::vec2& { return viewScale_; }
auto LayoutBase::getIndex() const -> uint32_t { return index_; }
auto LayoutBase::getAngle() const -> float { return angle_; }
auto LayoutBase::isCustomIndex() const -> bool { return isCustomIndex_; }

auto LayoutBase::setType(Type val) -> LayoutBase& { layoutType_ = val; return *this; }
auto LayoutBase::setMargin(const TBLR& val) -> LayoutBase& { margin = val; return *this; }
auto LayoutBase::setPadding(const TBLR& val) -> LayoutBase& { padding = val; return *this; }
auto LayoutBase::setBorder(const TBLR& val) -> LayoutBase& { border = val; return *this; }
auto LayoutBase::setBorderRadius(const TBLR& val) -> LayoutBase& { borderRadius = val; return *this;}
auto LayoutBase::setShadow(const TBLR& val) -> LayoutBase& { shadow = val; return *this; }
auto LayoutBase::setWrap(const bool val) -> LayoutBase& { wrap = val; return *this; }
auto LayoutBase::setPos(const PositionXY& val) -> LayoutBase& { userPos_ = val; return *this; }
auto LayoutBase::setScale(const ScaleXY& val) -> LayoutBase& { userScale_ = val; return *this; }
auto LayoutBase::setComputedPos(const glm::vec2& val) -> LayoutBase& { computedPos_ = val; return *this; }
auto LayoutBase::setComputedScale(const glm::vec2& val) -> LayoutBase& {computedScale_ = val;return *this;}
auto LayoutBase::setViewPos(const glm::vec2& val) -> LayoutBase& { viewPos_ = val; return *this; }
auto LayoutBase::setViewScale(const glm::vec2& val) -> LayoutBase& { viewScale_ = val; return *this; }
auto LayoutBase::setIndex(uint32_t val) -> LayoutBase& { index_ = val;  return *this; }
auto LayoutBase::setEnableCustomIndex(const bool val) -> LayoutBase& { isCustomIndex_ = val;  return *this; }
auto LayoutBase::setAngle(float val) -> LayoutBase& { angle_ = val; return *this; }

LayoutBase::Scale operator"" _fill(unsigned long long)
{
    return {.val = 1.0f, .type = LayoutBase::ScaleType::FILL};
}

LayoutBase::Scale operator"" _fit(unsigned long long)
{
    return {.val = 1.0f, .type = LayoutBase::ScaleType::FIT};
}

LayoutBase::Scale operator"" _px(unsigned long long value)
{
    /* Loss of precision justified. */
    return {.val = (float)value, .type = LayoutBase::ScaleType::PX};
}

LayoutBase::Scale operator"" _rel(long double value)
{
    /* Loss of precision justified. */
    return {.val = (float)value, .type = LayoutBase::ScaleType::REL};
}
} // namespace src::elementcomposable