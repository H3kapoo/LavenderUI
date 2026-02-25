#include <LavenderUI/Core/TextHandler/TextBatchStore.hpp>

namespace lav::core
{
auto TextBatchStore::get() -> TextBatchStore&
{
    static TextBatchStore instance;
    return instance;
}

TextBatchStore::TextBatchStore()
    : glyphModel_()
    , glyphCode_()
    , limit_(128)
    , globalOffset_(0)
    , batchesCount_(0)
{

    setLimit(limit_);
}

auto TextBatchStore::start() -> void
{
    globalOffset_ = 0;
    batchesCount_ = 0;
    clearBuffer();
}

auto TextBatchStore::end() -> void
{
    batchesCount_ += glyphCode_.empty() || isFull() ? 0 : 1;
}

auto TextBatchStore::clearBuffer() -> void
{
    glyphCode_.clear();
    glyphModel_.clear();
}

auto TextBatchStore::push(const int32_t code, glm::mat4&& model) -> void
{
    glyphCode_.emplace_back(code);
    glyphModel_.emplace_back(std::move(model));

    if (isFull()) { ++batchesCount_; }
}

auto TextBatchStore::setLimit(const uint32_t limit) -> void
{
    limit_ = limit;
    glyphCode_.reserve(limit);
    glyphModel_.reserve(limit);
}

auto TextBatchStore::setGlobalOffset(const uint32_t offset) -> void
{
    globalOffset_ = offset;
}

auto TextBatchStore::isFull() const -> bool
{
    return glyphCode_.size() == limit_ && glyphModel_.size() == limit_;
}

auto TextBatchStore::getModels() const -> const TextModelVec&
{
    return glyphModel_;
}

auto TextBatchStore::getGlyphs() const -> const TextGlyphVec&
{
    return glyphCode_;
}

auto TextBatchStore::getCurrentBatchSize() const -> uint32_t
{
    return glyphCode_.size();
}

auto TextBatchStore::getLimit() const -> uint32_t
{
    return limit_;
}

auto TextBatchStore::getGlobalOffset() const -> uint32_t
{
    return globalOffset_;
}

auto TextBatchStore::getBatchesCount() const -> uint32_t
{
    return batchesCount_;
}
} // namespace lav::core