#include "UIImage.hpp"

#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Core/LayoutHandler/BasicCalculator.hpp"
#include "src/Core/ResourceHandler/TextureLoader.hpp"

namespace lav::node
{
UIImage::UIImage()
    : UIBase({"UIImage", "elemVert.glsl", "elemFrag.glsl" })
{}

auto UIImage::render(const glm::mat4& projection) -> void
{
    glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", color);
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    shader_.uploadVec4f("uBorderRadii", layoutBase_.getBorderRadius());
    shader_.uploadInt("uUseTexture", imgTexData_.id);
    shader_.uploadTexture2D("uTexture", 0, imgTexData_.id);
    core::GPUBinder::get().renderBoundQuad();
}

auto UIImage::layout() -> void
{
    const auto& calculator = core::BasicCalculator::get();
    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForGenericElement(this);
}

auto UIImage::event(UIStatePtr& state) -> void
{

}

auto UIImage::setImage(const std::filesystem::path& path) -> bool
{
    imgTexData_ = core::TextureLoader::get().load(path, {});
    return imgTexData_.id ? true : false;
}

// auto UIImage::setColor(const glm::vec4& value) -> UIButton& { baseColor_ = value; return *this; }
// auto UIImage::setBorderColor(const glm::vec4& value) -> UIButton& { borderColor_ = value; return *this; }
// auto UIImage::getColor() const -> const glm::vec4& { return baseColor_; }
// auto UIImage::getBorderColor() const -> const glm::vec4& { return borderColor_; }

} // namespace lav::node
