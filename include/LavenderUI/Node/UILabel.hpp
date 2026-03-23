#pragma once

#include <optional>

#include <LavenderUI/Core/ResourceHandler/Font.hpp>
#include <LavenderUI/Core/TextHandler/Common.hpp>
#include <LavenderUI/Node/UIBase.hpp>

namespace lav::node
{
/**
 * @class Helps display a line of text.
 *
 * Text can be word/character wrapped, aligned and ending ellipsis dots can be added if needed.
 * This does not support true multiline and anything that looks like multiple lines happens due to
 * wrap settings.
 * This does not support vertical text orientation.
 * Implementation wise the text is rendered in batches for efficiency reasons.
 * Internally we do keep track of "per line data" just for text alignment reasons.
 */

class UILabel : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UILabel, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UILabel);

    /**
     * @fn Sets the viewable text.
     *
     * @param text Text to be rendered.
     */
    auto setText(const std::string& text) -> void;

    /**
     * @fn Sets the text font with optional font size.
     *
     * @note If this fails, font won't be changed.
     *
     * @param fontPath Relative path to the font file.
     * @param size Font size in pixels.
     */
    auto setFont(const fs::path& fontPath, const uint32_t size = 16) -> void;

    /**
     * @fn Sets the font's size.
     *
     * @param size Font size in pixels.
     */
    auto setFontSize(const uint32_t size) -> void;

    /**
     * @fn Sets text's color.
     *
     * @param color New text color.
     */
    auto setTextColor(const glm::vec4& color) -> void;

    /**
     * @fn Sets text alignment.
     *
     * @param align Side to which to align the text.
     */
    auto setTextAlign(const core::TextOptions::Align align) -> void;

    /**
     * @fn Sets wrapping method to be applied.
     *
     * @param wrap Wrapping method to be used.
     */
    auto setTextWrap(const core::TextOptions::Wrap wrap) -> void;

    /**
     * @fn Sets ellipsis (...) count at the end of available rendering space.
     *
     * @param count Sets how many dots (ellipsis) need to be added.
     */
    auto setTextEllipsis(const uint32_t count) -> void;

    /**
     * @fn Get copy of set text.
     *
     * @return Copied text.
     */
    auto getText() const -> std::string;

    /**
     * @fn Get copy of set text color.
     *
     * @return Copied text color.
     */
    auto getTextColor() const -> glm::vec4;

protected:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(core::UIStatePtr& state) -> void override;

    auto batchText(const glm::mat4& projection) -> void;

    struct LineData
    {
        uint32_t startIdx{0};
        uint32_t endIdx{0};
        uint32_t length{0};
        uint32_t lineIdx{0};
    };

private:
    using TextModelVec = std::vector<glm::mat4>;
    using TextGlyphVec = std::vector<int32_t>;

    auto renderBatch() -> void;
    auto handleLine(const LineData& ld) -> void;
    auto handleEllipsis(glm::ivec2& basePos) -> void;
    auto pushCharData(const core::Font::GlyphData& data, const glm::ivec2 pos) -> void;
    auto advanceBasePosition(const core::Font::GlyphData& data,
        glm::ivec2& basePos) -> void;
    auto computeAvailableTextBounds() -> void;
    auto computeInternalData() -> void;
    auto prepareBasePositionForLine(const LineData& ld) -> glm::ivec2;
    auto getMaxBoundPos() const -> glm::ivec2;

    /* Batching */
    auto startTextBatching() -> void;
    auto endTextBatching() -> void;
    auto clearTextBuffer() -> void;
    auto isTextBatchFull() const -> bool;
    auto getCurrentBatchSize() const -> uint32_t;

protected:
    core::TextOptions options_;
    core::Shader textShader_;
    core::FontPtr font_;
    std::vector<LineData> lineData_;
    std::optional<glm::vec4> overrideColor_;
    std::string storedText_;
    glm::vec4 textColor_;
    glm::ivec2 textRenderBoundStart_;
    glm::ivec2 textRenderBoundScale_;
    glm::ivec2 lastCharPos_;
    glm::uvec2 maxLineDataXY_;

private:
    /* Batching */
    static TextModelVec glyphModel_;
    static TextGlyphVec glyphCode_;
    static uint32_t batchLimit_;
    static uint32_t batchesCount_;
};
using UILabelPtr = std::shared_ptr<UILabel>;
using UILabelWPtr = std::weak_ptr<UILabel>;
using UILabelPtrVec = std::vector<UILabelPtr>;
} // namespace lav::node
