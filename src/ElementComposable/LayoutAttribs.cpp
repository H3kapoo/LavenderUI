#include "LayoutAttribs.hpp"

#include "vendor/glm/gtc/matrix_transform.hpp"
#include "vendor/glm/trigonometric.hpp"

namespace src::elementcomposable
{
auto LayoutAttribs::isPointInside(const glm::ivec2& p) const -> bool
{
    return (p.x >= cPos.x && p.x <= cPos.x + cScale.x)
        && (p.y >= cPos.y && p.y <= cPos.y + cScale.y);
}

auto LayoutAttribs::isPointInsideView(const glm::ivec2& p) const -> bool
{
    return (p.x >= viewPos.x && p.x <= viewPos.x + viewScale.x)
        && (p.y >= viewPos.y && p.y <= viewPos.y + viewScale.y);
}

auto LayoutAttribs::computeViewBox(const LayoutAttribs& parentAttribs) -> void
{
    /* Take the intersection between this object's pos+scale & parent's already computed
        viewable area box.*/
    viewPos = {
        std::max(parentAttribs.viewPos.x, cPos.x),
        std::max(parentAttribs.viewPos.y, cPos.y)
    };

    const glm::vec2 pComposed = parentAttribs.viewPos + parentAttribs.viewScale;
    const glm::vec2 thisComposed = cPos + cScale;
    viewScale = {
        std::max(0.0f, std::min(pComposed.x, thisComposed.x) - viewPos.x),
        std::max(0.0f, std::min(pComposed.y, thisComposed.y) - viewPos.y)
    };
}

auto LayoutAttribs::getTransform() -> const glm::mat4&
{
    transform_ = glm::mat4{1.0f};
    transform_ = glm::translate(transform_, glm::vec3(cPos, index));
    // transform_ = glm::rotate(transform_, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    transform_ = glm::scale(transform_, glm::vec3(cScale, 1.0f));
    return transform_;
}

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