#include <LavenderUI/Node/InternalUse/UIViewItem.hpp>

#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/LayoutHandler/LayoutBase.hpp>
#include <LavenderUI/Core/LayoutHandler/BaseCalculator.hpp>
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::node
{
UIViewItem::UIViewItem(UIBaseInitData&& initData)
    : UIBase(std::move(initData))
    , button1_(utils::make<UIButton>())
    , button2_(utils::make<UIButton>())
{
    /* This plays more like a container, not a logic resolver. */
    // setIgnoreEvents();

    button1_->getBaseLayoutData().setScale({20_px, 1_fill});
    button2_->getBaseLayoutData().setScale({1_fill});

    button1_->setColor(utils::hexToVec4("#df72dfff"));
    button2_->setColor(utils::hexToVec4("#9c649cff"));

    // TODO: UIBase::add is broken when created with a vector
    // UIBase::add({button1_, button2_});
    UIBase::add(button1_);
    UIBase::add(button2_);
}

auto UIViewItem::onRender(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", baseColor_);
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    shader_.uploadVec4f("uBorderRadii", layoutBase_.getBorderRadius());
    shader_.uploadVec4f("uBorderColor", borderColor_);
    shader_.uploadInt("uUseTexture", 0);
    core::GPUBinder::get().renderBoundQuad();
}

auto UIViewItem::onLayout() -> void
{
    const auto& calculator = core::BaseCalculator::get();

    calculator.calculateScaleForGenericElement(this, {0, 0});
    calculator.calculatePositionForGenericElement(this, {0, 0});
}

auto UIViewItem::onEvent(core::UIStatePtr&) -> void
{}

auto UIViewItem::getButtonPair() -> std::pair<UIButtonPtr, UIButtonPtr>
{
    return {button1_, button2_};
}
} // namespace lav::node
