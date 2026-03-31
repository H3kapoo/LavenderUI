#include <LavenderUI/Node/UILabel.hpp>

#include <algorithm>
#include <functional>
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
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::node
{
UILabel::TextModelVec UILabel::glyphModel_ = {};
UILabel::TextGlyphVec UILabel::glyphCode_ = {};
uint32_t UILabel::batchLimit_ = 128;
uint32_t UILabel::batchesCount_ = 0;

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
    , textAvailBounds_()
    , overrideColor_(std::nullopt)
    , storedText_()
    , textColor_(utils::hexToVec4("#141414ff"))
    , lastCharPos_(0, 0)
    , maxLineDataXY_(0, 0)
    , notAllTextVisible_(false)
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
    /*
        Compute the bounding box in which the text characters can be arranged.
        Compute the internal lines which can occur due to wrapping. Data used for alignments.
        Compute maximum length on each axis for the computed internal lines.
    */
    textAvailBounds_ = computeAvailableTextBounds();
    computeInternalLineData();
    maxLineDataXY_ = computeInternalLineDataMax();
}

auto UILabel::onEvent(core::UIStatePtr& state) -> void
{
    ss_ = state->windowSize;
    /* Event processing starts here. */
    UIBase::processAndEmitGenericMouseEvents(state);

    if (state->currentEventId == core::MouseLeftReleaseEvt::eventId)
    {
        for (const auto ld : lineData_)
        {
            log_.info("Start {} length {}", ld.startIdx, ld.endIdx);
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

    /*
        Each line will be handled separately but one batch could cover more lines. This is done so
        that we can individualy align lines if wrap is enabled.
    */
    startTextBatching();
    for (uint32_t i = 0; i < lineData_.size(); ++i) { handleLine(lineData_[i], i); }
    endTextBatching();
}

auto UILabel::renderBatch() -> void
{
    const auto size = glyphCode_.size();
    if (!size) { return; }

    /* Render current batch. */
    textShader_.uploadIntv("uCharIndices", glyphCode_);
    textShader_.uploadMat4v("uModelMatrices", glyphModel_);
    core::GPUBinder::get().renderBoundQuadInstanced(size);
    clearTextBuffer();
}

auto UILabel::handleLine(const LineData& ld, const uint32_t lineIdx) -> void
{
    const bool isLastLine = lineIdx + 1 == lineData_.size(); 
    glm::ivec2 lineStart = computeLineStartPos(lineIdx, ld.length);
    bool ellipsisNeeded{false};

    for (uint32_t idx = ld.startIdx; idx < ld.endIdx; ++idx)
    {
        const core::Font::GlyphData& gd = font_->glyphData[storedText_[idx]];

        if (ellipsisNeeded = isEllipsisNeeded(isLastLine, gd, lineStart); ellipsisNeeded) { break; }

        /* Ajust char into position. */
        glm::ivec2 pos = lineStart;
        pos.x += gd.bearing.x;
        pos.y -= gd.bearing.y;

        lineStart.x += gd.hAdvance;

        pushDataAndRenderIfFull(gd, pos);
    }

    if (ellipsisNeeded) { handleEllipsis(lineStart); };

    /* Render last batch. */
    if (isLastLine) { renderBatch(); }
}

auto UILabel::handleEllipsis(glm::ivec2& lineStart) -> void
{
    const core::Font::GlyphData& gd = font_->glyphData['.'];
    for (uint32_t el = 0; el < options_.ellipsis; ++el)
    {
        /* Ajust char into position. */
        glm::ivec2 pos = lineStart;
        pos.x += gd.bearing.x;
        pos.y -= gd.bearing.y;

        lineStart.x += gd.hAdvance;
        pushDataAndRenderIfFull(gd, pos);
    }
}

auto UILabel::pushDataAndRenderIfFull(const core::Font::GlyphData& data, const glm::ivec2 pos) -> void
{
    /* Skip rendering any spaces. */
    if (data.glyphCode == ' ') { return; }

    glm::mat4 model{glm::mat4(1.0f)};
    model = glm::translate(model, glm::vec3(pos.x, pos.y, layoutBase_.getZIndex() + 0.01f));
    model = glm::scale(model, glm::vec3(font_->fontSize, font_->fontSize, 1));

    glyphCode_.emplace_back(data.glyphCode);
    glyphModel_.emplace_back(std::move(model));

    if (isTextBatchFull())
    {
        ++batchesCount_;
        renderBatch();
    }
}

auto UILabel::computeAvailableTextBounds() const -> TextAvailBounds
{
    const glm::ivec2 padStartPush = {
        layoutBase_.getPadding().left + layoutBase_.getBorder().left,
        layoutBase_.getPadding().top  + layoutBase_.getBorder().top};
    const glm::ivec2 padEndPop = {
        padStartPush.x + layoutBase_.getPadding().right + layoutBase_.getBorder().right,
        padStartPush.y + layoutBase_.getPadding().bot + layoutBase_.getBorder().bot};

    TextAvailBounds bounds;
    bounds.startPos = layoutBase_.getComputedPos() + padStartPush;
    bounds.scale = layoutBase_.getComputedScale() - padEndPop;
    bounds.endPos  = bounds.startPos + bounds.scale;

    return bounds;
}

auto UILabel::computeInternalLineData() -> void
{
    // here we can compute if ellipsis is really needed
    lineData_.clear();

    glm::ivec2 rawPos{textAvailBounds_.startPos};
    LineData ld{0, 0};

    for (uint32_t idx = 0; idx < storedText_.size(); ++idx)
    {
        const auto& gd = font_->glyphData[storedText_[idx]];
        glm::ivec2 pos;
        pos.x = rawPos.x + gd.bearing.x;
        pos.y = rawPos.y - gd.bearing.y;

        /* If this is true, current char will be placed on a new line. */
        if (options_.wrap == core::TextOptions::Wrap::CHAR
            && pos.x + gd.size.x > textAvailBounds_.endPos.x)
        {
            rawPos.x = textAvailBounds_.startPos.x;
            rawPos.y += font_->baseVerticalSep;
            pos = rawPos;

            ld.endIdx = idx;

            lineData_.push_back(ld);
            ld.startIdx = idx;
            ld.length = 0;
        }

        /* Stop if line is outside renderable zone. */
        if (pos.y + gd.size.y - gd.bearing.y > textAvailBounds_.endPos.y - font_->baseVerticalSep)
        {
            notAllTextVisible_ = true;
            return;
        }

        rawPos += glm::ivec2{gd.hAdvance, 0};
        ld.length += gd.hAdvance;
    }

    notAllTextVisible_ = false;
    ld.endIdx = storedText_.size();
    lineData_.push_back(ld);
}

auto UILabel::computeInternalLineDataMax() const -> glm::ivec2
{
    if (lineData_.empty()) { return {0, 0}; }

    return {
        std::ranges::max_element(lineData_, std::ranges::greater{}, &LineData::length)->length,
        lineData_.size() * font_->baseVerticalSep
    };
}

auto UILabel::computeLineStartPos(const uint32_t lineIdx, const int32_t lineLen) -> glm::ivec2
{
    glm::ivec2 lineStart{textAvailBounds_.startPos};
    lineStart.y += font_->descender + font_->baseVerticalSep * (lineIdx + 1);
    lineStart.y += (textAvailBounds_.scale.y - maxLineDataXY_.y) / 2;

    switch (options_.align)
    {
        case core::TextOptions::Align::LEFT:
            break;
        case core::TextOptions::Align::CENTER:
            lineStart.x += (textAvailBounds_.scale.x - lineLen) / 2;
            break;
        case core::TextOptions::Align::RIGHT:
            break;
        default:
            log_.warn("Could not align!");
    }

    return lineStart;
}

auto UILabel::isEllipsisNeeded(const bool isLast, const core::Font::GlyphData& data,
    const glm::ivec2& pos) -> bool
{
    if (options_.ellipsis <= 0) { return false; }
    if (options_.wrap != core::TextOptions::Wrap::NONE && !notAllTextVisible_) { return false; }
    if (!isLast) { return false; }

    const uint32_t dotAdvance = font_->glyphData['.'].hAdvance;
    const int32_t ellipsisSize = dotAdvance * options_.ellipsis;
    const int32_t positionAfterAdvance = pos.x + data.hAdvance;
    if (textAvailBounds_.endPos.x - positionAfterAdvance <= ellipsisSize)
    {
        return true;
    }

    return false;
}

auto UILabel::startTextBatching() -> void
{
    batchesCount_ = 0;
    clearTextBuffer();
}

auto UILabel::isTextBatchFull() const -> bool
{
    return glyphCode_.size() == batchLimit_ && glyphModel_.size() == batchLimit_;
}

auto UILabel::endTextBatching() -> void
{
    batchesCount_ += glyphCode_.empty() || isTextBatchFull() ? 0 : 1;
}

auto UILabel::clearTextBuffer() -> void
{
    glyphCode_.clear();
    glyphModel_.clear();
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
