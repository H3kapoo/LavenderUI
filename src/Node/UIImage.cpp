#include <LavenderUI/Node/UIImage.hpp>

#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/LayoutHandler/BaseCalculator.hpp>
#include <LavenderUI/Core/ResourceHandler/TextureLoader.hpp>
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::node
{
UIImage::UIImage(UIBaseInitData&& initData)
    : UIBase(std::move(initData))
    , imgTexData_()
{
    layoutBase_.setScale({200_px, 50_px});
}

auto UIImage::onRender(const glm::mat4& projection) -> void
{
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", baseColor_);
    shader_.uploadVec4f("uBorderColor", borderColor_);
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    shader_.uploadVec4f("uBorderRadii", layoutBase_.getBorderRadius());
    shader_.uploadInt("uUseTexture", imgTexData_.id);
    shader_.uploadTexture2D("uTexture", 0, imgTexData_.id);
    core::GPUBinder::get().renderBoundQuad();
}

auto UIImage::onLayout() -> void
{
    const auto& calculator = core::BaseCalculator::get();
    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForGenericElement(this);
}

auto UIImage::onEvent(core::UIStatePtr&) -> void
{}

auto UIImage::setImage(const std::filesystem::path& path) -> bool
{
    imgTexData_ = core::TextureLoader::get().load(path, {});
    return imgTexData_.id ? true : false;
}
} // namespace lav::node
