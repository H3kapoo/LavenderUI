#include <LavenderUI/Node/UILabel.hpp>

#include <optional>

#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/EventHandler/IEvent.hpp>
#include "LavenderUI/Core/Config.hpp"
#include "LavenderUI/Core/LayoutHandler/LayoutBase.hpp"
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::node
{
UILabel::UILabel(UIBaseInitData&& data)
    : UIBase(std::move(data))
    , textHandler_(
        core::Config::shadersPath / "basicTextVert.glsl",
        core::Config::shadersPath / "basicTextFrag.glsl")
    , overrideColor_(std::nullopt)
{
    layoutBase_.setScale({200_px, 50_px});
    textHandler_.setWrapEnabled(true);
    setIgnoreEvents();
}

auto UILabel::onRender(const glm::mat4& projection) -> void
{
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", overrideColor_ ? *overrideColor_ : getColor());
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    shader_.uploadVec4f("uBorderRadii", layoutBase_.getBorderRadius());
    shader_.uploadVec4f("uBorderColor", getBorderColor());
    shader_.uploadInt("uUseTexture", 0);
    core::GPUBinder::get().renderBoundQuad();

    /* Draw the text */
    textHandler_.render(projection);
}

auto UILabel::onLayout() -> void
{
    const glm::ivec2 padStartPush = glm::ivec2{layoutBase_.getPadding().left, layoutBase_.getPadding().top};
    const glm::ivec2 padEndPop = glm::ivec2{
        padStartPush.x + layoutBase_.getPadding().right,
        padStartPush.y + layoutBase_.getPadding().bot};
    const glm::ivec2 boundsStart = layoutBase_.getComputedPos() + padStartPush;
    const glm::ivec2 boundsScale = layoutBase_.getComputedScale() - padEndPop;

    textHandler_.setDisplayBounds({boundsStart, boundsScale, layoutBase_.getZIndex()});
}

auto UILabel::onEvent(core::UIStatePtr& state) -> void
{
    UIBase::processAndEmitGenericMouseEvents(state);
}

auto UILabel::setText(const std::string& text) -> void
{
    textHandler_.setText(text);
}

auto UILabel::setFont(const std::filesystem::path& fontPath) -> void
{
    textHandler_.setFont(fontPath);
}

auto UILabel::setTextColor(const glm::vec4& color) -> void
{
    textHandler_.setTextColor(color);
}

auto UILabel::setTextAlign(const core::TextOptions::Align align) -> void
{
    textHandler_.setTextAlign(align);
}

auto UILabel::setTextEllipsis(const uint32_t count) -> void
{
    textHandler_.setEllipsisEnabled(count);
}

auto UILabel::setScaleToTextSize() -> void
{
    layoutBase_.setScale({
        layoutBase_.getScale().x,
        {static_cast<float>(textHandler_.getFont().baseVerticalSep), core::LayoutBase::ScaleType::PX}
    });
}

auto UILabel::getText() const -> std::string
{
    return textHandler_.getText();
}

auto UILabel::getTextColor() const -> glm::vec4
{
    return textHandler_.getTextColor();
}

} // namespace src::uinodes
