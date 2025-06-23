#include "UIImage.hpp"

namespace src::uielements
{
UIImage::UIImage(const std::string& name)
    : UIBase(name, typeid(UIImage))
{}

auto UIImage::render(const glm::mat4& projection) -> void
{

}

auto UIImage::layout() -> void
{
}
} // namespace src::uielements
