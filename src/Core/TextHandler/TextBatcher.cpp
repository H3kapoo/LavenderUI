#include <LavenderUI/Core/TextHandler/TextBatcher.hpp>

namespace lav::core
{
auto TextBatcher::get() -> TextBatcher&
{
    static TextBatcher instance;
    return instance;
}

TextBatcher::TextBatcher()
    : glyphModel_()
    , glyphCode_()
    , limit_(128)
    , totalCount_(0)
{

    setLimit(limit_);
}

auto TextBatcher::reset() -> void
{
    totalCount_ = 0;
    clearBuffer();
}

auto TextBatcher::clearBuffer() -> void
{
    glyphCode_.clear();
    glyphModel_.clear();
}

auto TextBatcher::push(const int32_t code, glm::mat4&& model) -> void
{
    glyphCode_.emplace_back(code);
    glyphModel_.emplace_back(std::move(model));
    ++totalCount_;
}

auto TextBatcher::setLimit(const uint32_t limit) -> void
{
    limit_ = limit;
    glyphCode_.reserve(limit);
    glyphModel_.reserve(limit);
}

auto TextBatcher::isFull() const -> bool
{
    return glyphCode_.size() == limit_ && glyphModel_.size() == limit_;
}

auto TextBatcher::getModels() const -> const TextModelVec&
{
    return glyphModel_;
}

auto TextBatcher::getGlyphs() const -> const TextGlyphVec&
{
    return glyphCode_;
}

auto TextBatcher::getCurrentBatchSize() const -> uint32_t
{
    return glyphCode_.size();
}

auto TextBatcher::getLimit() const -> uint32_t
{
    return limit_;
}

auto TextBatcher::getTotalCount() const -> uint32_t
{
    return totalCount_;
}
} // namespace lav::core