#include "LayoutAttribs.hpp"

#include "vendor/glm/gtc/matrix_transform.hpp"
#include "vendor/glm/trigonometric.hpp"

namespace src::uielements
{
auto LayoutAttribs::computeViewBox(const LayoutAttribs& parentAttribs) -> void
{
    /* Take the intersection between this object's pos+scale & parent's already computed
        viewable area box.*/
    viewPos = {
        std::max(parentAttribs.viewPos.x, pos.x),
        std::max(parentAttribs.viewPos.y, pos.y)
    };

    const glm::vec2 pComposed = parentAttribs.viewPos + parentAttribs.viewScale;
    const glm::vec2 thisComposed = pos + scale;
    viewScale = {
        std::max(0.0f, std::min(pComposed.x, thisComposed.x) - viewPos.x),
        std::max(0.0f, std::min(pComposed.y, thisComposed.y) - viewPos.y)
    };
}

auto LayoutAttribs::getTransform() -> const glm::mat4&
{
    transform_ = glm::mat4{1.0f};
    transform_ = glm::translate(transform_, glm::vec3(pos, index));
    // transform_ = glm::rotate(transform_, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    transform_ = glm::scale(transform_, glm::vec3(scale, 1.0f));
    return transform_;
}
} // namespace src::uielements