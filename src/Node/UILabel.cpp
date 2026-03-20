#include <LavenderUI/Node/UILabel.hpp>

#include <optional>
#include <glm/ext/matrix_transform.hpp>

#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/EventHandler/IEvent.hpp>
#include "LavenderUI/Core/Config.hpp"
#include "LavenderUI/Core/EventHandler/CoreEvents/MouseLeftRelease.hpp"
#include "LavenderUI/Core/LayoutHandler/LayoutBase.hpp"
#include "LavenderUI/Core/ResourceHandler/Font.hpp"
#include "LavenderUI/Core/ResourceHandler/ShaderLoader.hpp"
#include "LavenderUI/Core/ResourceHandler/FontLoader.hpp"
#include "LavenderUI/Core/TextHandler/Common.hpp"
#include "LavenderUI/Core/TextHandler/TextBatchStore.hpp"
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::node
{
UILabel::UILabel(UIBaseInitData&& data)
    : UIBase(std::move(data))
    , options_({
        .ellipsis = 0,
        .lineHeight = 1,
        .align = core::TextOptions::Align::LEFT,
        .wrap = core::TextOptions::Wrap::NONE })
    , textShader_(core::ShaderLoader::get().load(
        core::Config::shadersPath / "basicTextVert.glsl",
        core::Config::shadersPath / "basicTextFrag.glsl"))
    , font_(core::FontLoader::get().loadFont(core::DEFAULT_FONT_PATH))
    , lineData_()
    , overrideColor_(std::nullopt)
    , storedText_()
    , textColor_(utils::hexToVec4("#141414ff"))
    , textRenderBoundStart_(0, 0)
    , textRenderBoundScale_(0, 0)
    , lastCharPos_(0, 0)
    , maxLineDataXY_(0, 0)
{
    /* Set runtime defaults */
    layoutBase_.setScale({200_px, 50_px});
}

auto UILabel::onRender(const glm::mat4& projection) -> void
{
    /* Render label container. */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", overrideColor_ ? *overrideColor_ : getColor());
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    shader_.uploadVec4f("uBorderRadii", layoutBase_.getBorderRadius());
    shader_.uploadVec4f("uBorderColor", getBorderColor());
    shader_.uploadInt("uUseTexture", false);
    core::GPUBinder::get().renderBoundQuad();

    /* Draw the text. */
    batchText(projection);
}

auto UILabel::onLayout() -> void
{
    computeAvailableTextBounds();
    computeInternalData();
}

auto UILabel::onEvent(core::UIStatePtr& state) -> void
{
    /* Event processing starts here. */
    UIBase::processAndEmitGenericMouseEvents(state);

    if (state->currentEventId == core::MouseLeftReleaseEvt::eventId)
    {
        for (const auto ld : lineData_)
        {
            log_.info("Start {} length {}", ld.startIdx, ld.length);
        }
        log_.warn("Max line {}", maxLineDataXY_.x);
    }
}

auto UILabel::batchText(const glm::mat4& projection) -> void
{
    /* Do nothing if there ain't something to display. */
    if (storedText_.empty()) { return; }

    /* Setup base shader values. */
    mesh_.bind();
    textShader_.bind();
    textShader_.uploadVec4f("uColor", textColor_);
    textShader_.uploadMat4("uMatrixProjection", projection);
    textShader_.uploadTexture2DArray("uTextureArray", 0, font_->textureId);

    core::TextBatchStore::get().start();

    for (const LineData& lineData : lineData_)
    {
        handleLine(lineData);
    }

    core::TextBatchStore::get().end();

    /* Save where the last char was placed. */
    // lastCharPos_ = basePos;
}

auto UILabel::renderBatch() -> void
{
    const auto size = core::TextBatchStore::get().getCurrentBatchSize();
    if (!size) { return; }

    /* Render current batch. */
    textShader_.uploadIntv("uCharIndices", core::TextBatchStore::get().getGlyphs());
    textShader_.uploadMat4v("uModelMatrices", core::TextBatchStore::get().getModels());
    core::GPUBinder::get().renderBoundQuadInstanced(size);
    core::TextBatchStore::get().clearBuffer();
}

auto UILabel::handleLine(const LineData& ld) -> void
{
    const uint32_t ellipsisSize = (font_->glyphData['.'].hAdvance >> 6) * options_.ellipsis;
    glm::ivec2 basePos{prepareBasePositionForLine(ld)};
    bool ellipsisNeeded{false};

    for (uint32_t idx = ld.startIdx; idx < ld.endIdx; ++idx)
    {
        if (core::TextBatchStore::get().isFull()) { renderBatch(); }

        core::Font::GlyphData gd = font_->glyphData[storedText_[idx]];
        if (getMaxBoundPos().x - (basePos.x + (gd.hAdvance >> 6)) <= ellipsisSize)
        {
            ellipsisNeeded = true;
            break;
        }

        core::TextBatchStore::get().setGlobalOffset(idx);

        /* Ajust char into position. */
        glm::ivec2 pos = basePos;
        pos.x += gd.bearing.x;
        pos.y -= gd.bearing.y;

        advanceBasePosition(gd, basePos);

        /* Skip rendering any spaces, but take into account the advance. */
        if (gd.glyphCode == ' ') { continue; }

        pushCharData(gd, pos);
    }

    if (ellipsisNeeded) { handleEllipsis(basePos); };

    /* Render last batch. */
    if (ld.lineIdx + 1 == lineData_.size()) { renderBatch(); }
}

auto UILabel::handleEllipsis(glm::ivec2& basePos) -> void
{
    for (uint32_t el = 0; el < options_.ellipsis; ++el)
    {
        if (core::TextBatchStore::get().isFull()) { renderBatch(); }
        core::Font::GlyphData gd = font_->glyphData['.'];
        gd = font_->glyphData['.'];

        /* Ajust char into position. */
        glm::ivec2 pos = basePos;
        pos.x += gd.bearing.x;
        pos.y -= gd.bearing.y;

        advanceBasePosition(gd, basePos);
        pushCharData(gd, pos);
    }
}

auto UILabel::pushCharData(const core::Font::GlyphData& data, const glm::ivec2 pos) -> void
{
    glm::mat4 model{glm::mat4(1.0f)};
    model = glm::translate(model, glm::vec3(pos.x, pos.y, layoutBase_.getZIndex() + 0.01f));
    model = glm::scale(model, glm::vec3(font_->fontSize, font_->fontSize, 1));

    core::TextBatchStore::get().push(data.glyphCode, std::move(model));
}

auto UILabel::advanceBasePosition(const core::Font::GlyphData& data,
    glm::ivec2& basePos) -> void
{
    /* Advance is stored in 1/64ths of a pixel by FT lib for some reason. Need to bitshift right. */
    basePos += glm::ivec2{(data.hAdvance >> 6), 0};
}

auto UILabel::computeAvailableTextBounds() -> void
{
    const glm::ivec2 padStartPush = {
        layoutBase_.getPadding().left + layoutBase_.getBorder().left,
        layoutBase_.getPadding().top  + layoutBase_.getBorder().top};
    const glm::ivec2 padEndPop = {
        padStartPush.x + layoutBase_.getPadding().right + layoutBase_.getBorder().right,
        padStartPush.y + layoutBase_.getPadding().bot + layoutBase_.getBorder().bot};
    textRenderBoundStart_ = layoutBase_.getComputedPos() + padStartPush;
    textRenderBoundScale_ = layoutBase_.getComputedScale() - padEndPop;
}

auto UILabel::computeInternalData() -> void
{
    lineData_.clear();
    maxLineDataXY_ = {0, 0};

    glm::ivec2 rawPos{textRenderBoundStart_};
    LineData ld{0, 0};
    ld.startIdx = 0;

    for (uint32_t idx = 0; idx < storedText_.size(); ++idx)
    {
        const auto& gd = font_->glyphData[storedText_[idx]];
        glm::ivec2 pos;
        pos.x = rawPos.x + gd.bearing.x;
        pos.y = rawPos.y - gd.bearing.y;

        /* If this is true, current char will be placed on a new line. */
        if (options_.wrap == core::TextOptions::Wrap::CHAR
            && pos.x + gd.size.x > getMaxBoundPos().x)
        {
            rawPos.x = textRenderBoundStart_.x;
            rawPos.y += font_->baseVerticalSep;
            pos = rawPos;

            maxLineDataXY_.x = std::max(maxLineDataXY_.x, ld.length);

            ld.endIdx = idx;

            lineData_.push_back(ld);
            ld.startIdx = idx;
            ld.length = 0;
            ld.lineIdx++;
        }

        rawPos += glm::ivec2{(gd.hAdvance >> 6), 0};
        ld.length += (gd.hAdvance >> 6);
    }

    ld.endIdx = storedText_.size();
    maxLineDataXY_.x = std::max(maxLineDataXY_.x, ld.length);
    lineData_.push_back(ld);
    maxLineDataXY_.y = lineData_.size() * font_->baseVerticalSep;
}

auto UILabel::prepareBasePositionForLine(const LineData& ld) -> glm::ivec2
{
    glm::ivec2 basePos{textRenderBoundStart_};
    basePos.y += font_->descender + font_->baseVerticalSep * (ld.lineIdx + 1);

    switch (options_.align)
    {
        case core::TextOptions::Align::LEFT:
            break;
        case core::TextOptions::Align::CENTER:
            basePos.x += (textRenderBoundScale_.x - ld.length) / 2;
            basePos.y += (textRenderBoundScale_.y - static_cast<int32_t>(maxLineDataXY_.y)) / 2;
            break;
        case core::TextOptions::Align::RIGHT:
            break;
        default:
            log_.warn("Could not align!");
    }

    return basePos;
}

auto UILabel::getMaxBoundPos() const -> glm::ivec2
{
    return textRenderBoundStart_ + textRenderBoundScale_;
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

auto UILabel::setTextWrap(const core::TextOptions::Wrap wrap) -> void
{
    options_.wrap = wrap;
}

auto UILabel::setTextEllipsis(const uint32_t count) -> void
{
    options_.ellipsis = count;
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
