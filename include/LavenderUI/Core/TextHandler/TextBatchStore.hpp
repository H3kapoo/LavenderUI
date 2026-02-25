#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace lav::core
{
/**
    @brief Class used to store batches of text so then it can be rendered one at the time.
        It also stores offset to know how much we "batched" so far and where the next batch
        shall start. Limit can be set for how big a batch can be. Default 128.
*/
class TextBatchStore
{
using TextModelVec = std::vector<glm::mat4>;
using TextGlyphVec = std::vector<int32_t>;

public:
    static auto get() -> TextBatchStore&;

    auto start() -> void;
    auto end() -> void;
    auto clearBuffer() -> void;
    auto push(const int32_t code, glm::mat4&& model) -> void;
    auto setLimit(const uint32_t limit) -> void;
    auto setGlobalOffset(const uint32_t offset) -> void;
    auto isFull() const -> bool;
    auto getModels() const -> const TextModelVec&;
    auto getGlyphs() const -> const TextGlyphVec&;
    auto getCurrentBatchSize() const -> uint32_t;
    auto getLimit() const -> uint32_t;
    auto getGlobalOffset() const -> uint32_t;
    auto getBatchesCount() const -> uint32_t;

private:
    TextBatchStore();
    ~TextBatchStore() = default;
    TextBatchStore(const TextBatchStore&) = delete;
    TextBatchStore(TextBatchStore&&) = delete;
    auto operator=(const TextBatchStore&) -> TextBatchStore& = delete;
    auto operator=(TextBatchStore&&) -> TextBatchStore& = delete;

private:
    TextModelVec glyphModel_;
    TextGlyphVec glyphCode_;
    uint32_t limit_;
    uint32_t globalOffset_;
    uint32_t batchesCount_;
};
} // namespace lav::core