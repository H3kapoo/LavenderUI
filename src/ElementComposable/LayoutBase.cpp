#include "LayoutBase.hpp"

#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"
#include "vendor/glm/ext/vector_float2.hpp"
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

auto LayoutBase::computeOffsetToCenter(const glm::ivec2& p) const -> glm::ivec2
{
    const glm::vec2 center = computedPos_ + computedScale_ / 2;
    return p - center;
}

auto LayoutBase::distanceToCenter(const glm::ivec2& p) const -> float
{
    const glm::vec2 center = computedPos_ + computedScale_ / 2;
    // return std::sqrt(std::pow(p.x - center.x, 2) + std::pow(p.y - center.y, 2));
    return std::abs(p.x - center.x);
}

auto LayoutBase::computeViewBox(const LayoutBase& parentAttribs) -> void
{
    /* Take the intersection between this object's pos+scale & parent's already computed
        viewable area box minus parent borders.*/
    const glm::vec2 pBorderPos = { parentAttribs.getBorder().left, parentAttribs.getBorder().top };
    const glm::ivec2 pBorderScale = { parentAttribs.getLRBorder(), parentAttribs.getTBBorder()};

    viewPos_ = {
        std::max((float)parentAttribs.viewPos_.x + pBorderPos.x, computedPos_.x),
        std::max((float)parentAttribs.viewPos_.y + pBorderPos.y, computedPos_.y)
    };

    const glm::vec2 pViewEnd = parentAttribs.viewPos_ + pBorderPos + parentAttribs.viewScale_ - pBorderScale;
    const glm::vec2 thisEnd = computedPos_ + computedScale_;
    viewScale_ = {
        std::round(std::max(0.0f, std::min(pViewEnd.x, thisEnd.x) - viewPos_.x)),
        std::round(std::max(0.0f, std::min(pViewEnd.y, thisEnd.y) - viewPos_.y))
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
    return margin_.left + margin_.right;
}

auto LayoutBase::getTBMargin() const -> int32_t
{
    return margin_.top + margin_.bot;
}

auto LayoutBase::getLRBorder() const -> int32_t
{
    return border_.left + border_.right;
}

auto LayoutBase::getTBBorder() const -> int32_t
{
    return border_.top + border_.bot;
}

auto LayoutBase::getLRPadding() const -> int32_t
{
    return padding_.left + padding_.right;
}

auto LayoutBase::getTBPadding() const -> int32_t
{
    return padding_.top + padding_.bot;
}

auto LayoutBase::getFullBoxPos() const -> glm::vec2
{
    return {computedPos_.x - margin_.left, computedPos_.y - margin_.top};
}

auto LayoutBase::getFullBoxScale() const -> glm::vec2
{
    return {computedScale_.x + margin_.left + margin_.right, computedScale_.y + margin_.top + margin_.bot};
}

auto LayoutBase::getContentBoxPos() const -> glm::vec2
{
    return {computedPos_.x + padding_.left + border_.left, computedPos_.y + padding_.top + border_.top};
}

auto LayoutBase::getContentBoxScale() const -> glm::vec2
{
    return {computedScale_.x - padding_.left - padding_.right - border_.left - border_.right,
        computedScale_.y - padding_.top - padding_.bot - border_.top - border_.bot};
}

auto LayoutBase::getType() const -> Type { return layoutType_; }
auto LayoutBase::getMargin() const -> const TBLR& { return margin_; }
auto LayoutBase::getPadding() const -> const TBLR& { return padding_; }
auto LayoutBase::getBorder() const -> const TBLR& { return border_; }
auto LayoutBase::getBorderRadius() const -> const TBLR& { return borderRadius_; }
auto LayoutBase::getShadow() const -> const TBLR& { return shadow_; }
auto LayoutBase::getSelfAlign() const -> const Align& { return selfAlign_; }
auto LayoutBase::getAlign() const -> const Align& { return align_; }
auto LayoutBase::getSpacing() const -> const Spacing& { return spacing_; }
auto LayoutBase::getGrid() -> GridPolicyXY& { return gridPolicy_; }
auto LayoutBase::getGridPos() const -> GridRC { return gridPos_; }
auto LayoutBase::getGridSpan() const -> GridRC { return gridSpan_; }
auto LayoutBase::getMinScale() const -> const glm::ivec2& { return minScale_; }
auto LayoutBase::getMaxScale() const -> const glm::ivec2& { return maxScale_; }
auto LayoutBase::getWrap() const -> bool { return wrap; }
auto LayoutBase::getPos() const -> const PositionXY& { return userPos_; }
auto LayoutBase::getScale() const -> const ScaleXY& { return userScale_; }
auto LayoutBase::getComputedPos() const -> const glm::vec2& { return computedPos_; }
auto LayoutBase::getComputedScale() const -> const glm::vec2& { return computedScale_; }
auto LayoutBase::getViewPos() const -> const glm::ivec2& { return viewPos_; }
auto LayoutBase::getViewScale() const -> const glm::ivec2& { return viewScale_; }
auto LayoutBase::getIndex() const -> uint32_t { return index_; }
auto LayoutBase::getAngle() const -> float { return angle_; }
auto LayoutBase::isCustomIndex() const -> bool { return isCustomIndex_; }

auto LayoutBase::setType(Type val) -> LayoutBase& { layoutType_ = val; return *this; }
auto LayoutBase::setMargin(const TBLR& val) -> LayoutBase& { margin_ = val; return *this; }
auto LayoutBase::setPadding(const TBLR& val) -> LayoutBase& { padding_ = val; return *this; }
auto LayoutBase::setBorder(const TBLR& val) -> LayoutBase& { border_ = val; return *this; }
auto LayoutBase::setBorderRadius(const TBLR& val) -> LayoutBase& { borderRadius_ = val; return *this;}
auto LayoutBase::setShadow(const TBLR& val) -> LayoutBase& { shadow_ = val; return *this; }
auto LayoutBase::setSelfAlign(const Align val) -> LayoutBase& { selfAlign_ = val; return *this; }
auto LayoutBase::setAlign(const Align val) -> LayoutBase& { align_ = val; return *this; }
auto LayoutBase::setSpacing(const Spacing val) -> LayoutBase& { spacing_ = val; return *this; }
auto LayoutBase::setGrid(const GridPolicyXY value) -> LayoutBase& { gridPolicy_ = value; return *this; }
auto LayoutBase::setGridPos(const GridRC value) -> LayoutBase& { gridPos_ = value; return *this; }
auto LayoutBase::setGridSpan(const GridRC value) -> LayoutBase& { gridSpan_ = value; return *this; }
auto LayoutBase::setMinScale(const glm::ivec2 val) -> LayoutBase& { minScale_ = val; return *this; }
auto LayoutBase::setMaxScale(const glm::ivec2 val) -> LayoutBase& { maxScale_ = val; return *this; }
auto LayoutBase::setWrap(const bool val) -> LayoutBase& { wrap = val; return *this; }
auto LayoutBase::setPos(const PositionXY& val) -> LayoutBase& { userPos_ = val; return *this; }
auto LayoutBase::setScale(const ScaleXY& val) -> LayoutBase& { userScale_ = val; return *this; }
// auto LayoutBase::setComputedPos(const glm::vec2& val) -> LayoutBase& { computedPos_ = utils::round(val); return *this; }
// auto LayoutBase::setComputedScale(const glm::vec2& val) -> LayoutBase& {computedScale_ = utils::round(val) ;return *this;}
auto LayoutBase::setComputedPos(const glm::vec2& val) -> LayoutBase& { computedPos_ = val; return *this; }
auto LayoutBase::setComputedScale(const glm::vec2& val) -> LayoutBase& {computedScale_ = val ;return *this;}
auto LayoutBase::setViewPos(const glm::vec2& val) -> LayoutBase& { viewPos_ = val; return *this; }
auto LayoutBase::setViewScale(const glm::vec2& val) -> LayoutBase& { viewScale_ = val; return *this; }
auto LayoutBase::setIndex(uint32_t val) -> LayoutBase& { index_ = val;  return *this; }
auto LayoutBase::setEnableCustomIndex(const bool val) -> LayoutBase& { isCustomIndex_ = val;  return *this; }
auto LayoutBase::setAngle(float val) -> LayoutBase& { angle_ = val; return *this; }

LayoutBase::Scale operator"" _fill(unsigned long long)
{
    return LayoutBase::Scale{1.0f, LayoutBase::ScaleType::FILL};
}

LayoutBase::Scale operator"" _fit(unsigned long long)
{
    return LayoutBase::Scale{1.0f, LayoutBase::ScaleType::FIT};
}

LayoutBase::Scale operator"" _px(unsigned long long value)
{
    /* Loss of precision justified. */
    return LayoutBase::Scale{(float)value, LayoutBase::ScaleType::PX};
}

LayoutBase::Scale operator"" _rel(long double value)
{
    /* Loss of precision justified. */
    return LayoutBase::Scale{(float)value, LayoutBase::ScaleType::REL};
}

LayoutBase::Scale operator"" _fr(unsigned long long value)
{
    /* Loss of precision justified. */
    return LayoutBase::Scale{(float)value, LayoutBase::ScaleType::FR};
}

LayoutBase::Position operator"" _abs(unsigned long long value)
{
    return LayoutBase::Position(value, LayoutBase::PositionType::ABS);
}

LayoutBase::Position operator"" _comp(unsigned long long value)
{
    /* Won't really matter much as computed is the default and anything the user puts here is discared. */
    return LayoutBase::Position(value, LayoutBase::PositionType::COMPUTED);
}

auto operator-(const glm::vec2 lhs, const LayoutBase::TBLR rhs) -> glm::vec2
{
    return {lhs.x - (rhs.left + rhs.right), lhs.y - (rhs.top + rhs.bot)};
}

auto operator+(const glm::vec2 lhs, const glm::ivec2 rhs) -> glm::vec2
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

auto operator-(const glm::vec2 lhs, const glm::ivec2 rhs) -> glm::vec2
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

auto operator/(const glm::vec2 lhs, const int32_t rhs) -> glm::vec2
{
    return {lhs.x / rhs, lhs.y / rhs};
}

auto operator/(const glm::vec2 lhs, const glm::ivec2 rhs) -> glm::vec2
{
    return {lhs.x / rhs.x, lhs.y / rhs.y};
}
} // namespace src::elementcomposable