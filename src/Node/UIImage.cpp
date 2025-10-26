#include "UIImage.hpp"

namespace src::uielements
{
UIImage::UIImage()
    : UIBase(typeid(UIImage))
{}

auto UIImage::render(const glm::mat4& projection) -> void
{

}

auto UIImage::layout() -> void
{
}
} // namespace src::uielements
