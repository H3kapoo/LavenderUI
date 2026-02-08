#include "LavenderUI/Core/Config.hpp"
#include <LavenderUI/Node/UILabel.hpp>

#include <optional>

#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/EventHandler/IEvent.hpp>
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
    const glm::vec2 p = layoutBase_.getComputedPos() + layoutBase_.getComputedScale() / 2.0f
        - textHandler_.computeMaxSize() / 2.0f;

    textHandler_.setAnchorPos(p);
    textHandler_.setStartZIndex(layoutBase_.getZIndex());
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

auto UILabel::getText() const -> std::string
{
    return textHandler_.getText();
}
} // namespace src::uinodes
