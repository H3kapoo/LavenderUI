#include <LavenderUI/Node/UILabel.hpp>

#include <optional>

#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/EventHandler/IEvent.hpp>
#include "LavenderUI/Core/Config.hpp"
#include "LavenderUI/Core/LayoutHandler/LayoutBase.hpp"
#include "LavenderUI/Core/ResourceHandler/ShaderLoader.hpp"
#include "LavenderUI/Core/ResourceHandler/FontLoader.hpp"
#include "LavenderUI/Core/TextHandler/TextBatchStore.hpp"
#include <LavenderUI/Utils/Misc.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace lav::node
{
UILabel::UILabel(UIBaseInitData&& data)
    : UIBase(std::move(data))
    , textColor_(utils::hexToVec4("#141414ff"))
    , textShader_(core::ShaderLoader::get().load(
        core::Config::shadersPath / "basicTextVert.glsl",
        core::Config::shadersPath / "basicTextFrag.glsl"))
    , font_(core::FontLoader::get().loadFont(core::DEFAULT_FONT_PATH))
    , overrideColor_(std::nullopt)
{
    layoutBase_.setScale({200_px, 50_px});
    // textHandler_.setWrapEnabled(true);
    // setIgnoreEvents(); // TODO: Shall ignore events only when parented to a button
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
    handleText(projection);
}

auto UILabel::onLayout() -> void
{
    const glm::ivec2 padStartPush = glm::ivec2{layoutBase_.getPadding().left, layoutBase_.getPadding().top};
    const glm::ivec2 padEndPop = glm::ivec2{
        padStartPush.x + layoutBase_.getPadding().right,
        padStartPush.y + layoutBase_.getPadding().bot};
    const glm::ivec2 boundsStart = layoutBase_.getComputedPos() + padStartPush;
    const glm::ivec2 boundsScale = layoutBase_.getComputedScale() - padEndPop;

    layoutBounds_ = {boundsStart, boundsScale, layoutBase_.getZIndex()};

    basePos_ = layoutBounds_.pos;
    basePos_.y += font_->baseVerticalSep + font_->descender;
}

auto UILabel::onEvent(core::UIStatePtr& state) -> void
{
    UIBase::processAndEmitGenericMouseEvents(state);
}

auto UILabel::handleText(const glm::mat4& projection) -> void
{
    if (storedText_.empty()) { return; }

    /* Setup base shader values. */
    mesh_.bind();
    textShader_.bind();
    textShader_.uploadVec4f("uColor", textColor_);
    textShader_.uploadMat4("uMatrixProjection", projection);
    textShader_.uploadTexture2DArray("uTextureArray", 0, font_->textureId);

    // alignText();

    core::TextBatchStore::get().start();
    while (prepareNextBatch())
    {
        renderBatch();
    }
    core::TextBatchStore::get().end();

    /* Save where the last char was placed. */
    lastCharPos_ = basePos_;
}

auto UILabel::renderBatch() -> void
{
    core::TextBatchStore& tbs = core::TextBatchStore::get();
    textShader_.uploadIntv("uCharIndices", tbs.getGlyphs());
    textShader_.uploadMat4v("uModelMatrices", tbs.getModels());
    core::GPUBinder::get().renderBoundQuadInstanced(tbs.getCurrentBatchSize());
}

auto UILabel::prepareNextBatch() -> bool
{
    core::TextBatchStore& tbs = core::TextBatchStore::get();
    uint32_t globalCharIndex = tbs.getGlobalOffset();
    if (globalCharIndex >= storedText_.size() || !isPosInsideBounds(basePos_)) { return false; }

    /* Clear previous batch data */
    tbs.clearBuffer();

    while (!tbs.isFull())
    {
        /* Handle last batch which might not be full.*/
        if (globalCharIndex >= storedText_.size()) { return true; }

        const auto& gd = font_->glyphData[storedText_[globalCharIndex]];

        tbs.setGlobalOffset(++globalCharIndex);

        /* Ajust char into position. */
        glm::ivec2 pos;
        pos.x = basePos_.x + gd.bearing.x;
        pos.y = basePos_.y - gd.bearing.y;

        if (options_.wrap && pos.x + gd.size.x > layoutBounds_.pos.x + layoutBounds_.scale.x)
        {
            basePos_.y += font_->baseVerticalSep;
            basePos_.x = layoutBounds_.pos.x;
            pos = basePos_;
            pos.y -= gd.bearing.y;
        }

        /* Advance is stored in 1/64ths of a pixel by FT lib for some reason. Need to bitshift right. */
        basePos_ += glm::ivec2{(gd.hAdvance >> 6), 0};

        /* Skip rendering any spaces, but take into account the advance. */
        if (gd.glyphCode == ' ') { continue; }

        glm::mat4 model{glm::mat4(1.0f)};
        model = glm::translate(model, glm::vec3(pos.x, pos.y, layoutBase_.getZIndex() + 0.01f));
        model = glm::scale(model, glm::vec3(font_->fontSize, font_->fontSize, 1));

        tbs.push(gd.glyphCode, std::move(model));
    }

    return true;
}

auto UILabel::computeMaxTextBounds() -> void
{
    maxTextBounds_ = glm::ivec2{0, font_->baseVerticalSep};
    glm::ivec2 endPos{layoutBounds_.pos};
    for (const auto chr : storedText_)
    {
        const auto& gd = font_->glyphData[chr];
        glm::ivec2 pos;
        pos.x = endPos.x + gd.bearing.x;
        pos.y = endPos.y - gd.bearing.y;

        if (options_.wrap && pos.x + gd.size.x > layoutBounds_.pos.x + layoutBounds_.scale.x)
        {
            endPos.y += font_->baseVerticalSep;
            endPos.x = layoutBounds_.pos.x;
            pos = endPos;
            pos.y -= gd.bearing.y;

            maxTextBounds_.y += font_->baseVerticalSep;
        }

        /* Advance is stored in 1/64ths of a pixel by FT lib for some reason. Need to bitshift right. */
        endPos += glm::ivec2{(gd.hAdvance >> 6), 0};
        maxTextBounds_.x = std::max(maxTextBounds_.x, endPos.x - layoutBounds_.pos.x);
    }
}

auto UILabel::alignText() -> void
{
    switch (options_.align)
    {
        case core::TextOptions::Align::LEFT:
            break;
        case core::TextOptions::Align::CENTER:
        {
            computeMaxTextBounds();
            basePos_ += (layoutBounds_.scale - maxTextBounds_) / 2;
            break;
        }
        case core::TextOptions::Align::RIGHT:
            break;
    }
}

auto UILabel::isPosInsideBounds(const glm::ivec2 pos) -> bool
{
    return (pos.y <= layoutBounds_.pos.y + layoutBounds_.scale.y)
        && (pos.x <= layoutBounds_.pos.x + layoutBounds_.scale.x);
}

auto UILabel::setText(const std::string& text) -> void
{
    storedText_ = text;
}

auto UILabel::setFont(const std::filesystem::path& fontPath, const uint32_t size) -> void
{
    const core::FontPtr wantedFont = core::FontLoader::get().loadFont(fontPath, size);
    if (wantedFont->textureId) { font_ = wantedFont; }
}

auto UILabel::setFontSize(const uint32_t size) -> void
{
    setFont(font_->fontPath, size);
}

auto UILabel::setTextColor(const glm::vec4& color) -> void
{
    textColor_ = color;
}

auto UILabel::setTextAlign(const core::TextOptions::Align align) -> void
{
    options_.align = align;
}

auto UILabel::setTextWrap(const bool value) -> void
{
    options_.wrap = value;
}

auto UILabel::setTextEllipsis(const uint32_t count) -> void
{
    options_.ellipsis = count;
}

auto UILabel::setScaleToTextSize() -> void
{
    const float lineHeight = font_->baseVerticalSep;
    layoutBase_.setScale(
    {
        layoutBase_.getScale().x,
        {lineHeight, core::LayoutBase::ScaleType::PX}
    });
}

auto UILabel::getText() const -> std::string
{
    return storedText_;
}

auto UILabel::getTextColor() const -> glm::vec4
{
    return textColor_;
}

} // namespace src::uinodes
