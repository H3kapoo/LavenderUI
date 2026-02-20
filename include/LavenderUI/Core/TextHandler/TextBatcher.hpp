#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace lav::core
{
class TextBatcher
{
using TextModelVec = std::vector<glm::mat4>;
using TextGlyphVec = std::vector<int32_t>;

public:
    static auto get() -> TextBatcher&;

    auto reset() -> void;
    auto clearBuffer() -> void;
    auto push(const int32_t code, glm::mat4&& model) -> void;
    auto setLimit(const uint32_t limit) -> void;
    auto isFull() const -> bool;
    auto getModels() const -> const TextModelVec&;
    auto getGlyphs() const -> const TextGlyphVec&;
    auto getCurrentBatchSize() const -> uint32_t;
    auto getLimit() const -> uint32_t;
    auto getTotalCount() const -> uint32_t;

private:
    TextBatcher();
    ~TextBatcher() = default;
    TextBatcher(const TextBatcher&) = delete;
    TextBatcher(TextBatcher&&) = delete;
    auto operator=(const TextBatcher&) -> TextBatcher& = delete;
    auto operator=(TextBatcher&&) -> TextBatcher& = delete;

private:
    TextModelVec glyphModel_;
    TextGlyphVec glyphCode_;
    uint32_t limit_;
    uint32_t totalCount_;
};
} // namespace lav::core