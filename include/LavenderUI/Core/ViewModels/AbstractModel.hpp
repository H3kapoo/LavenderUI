#pragma once

#include <cstdint>
#include <memory>

#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"

namespace lav::core
{
struct ModelIndex
{
    uint32_t row{UINT32_MAX};
    uint32_t column{UINT32_MAX};
    void* internalPtr{nullptr};

    ModelIndex() {}
    ModelIndex(uint32_t r) : row(r) {}
    ModelIndex(uint32_t r, uint32_t c, void* ip)
        : row(r)
        , column(c)
        , internalPtr(ip) {}
    ~ModelIndex() = default;

    bool isValid() const { return row != UINT32_MAX && column != UINT32_MAX; }

    auto operator==(const ModelIndex& other) const -> bool
    {
        return other.row == row && other.column == column && other.internalPtr == internalPtr;
    }
};

struct ModelIndexHash
{
    std::size_t operator()(const ModelIndex& idx) const
    {
        std::size_t h1 = std::hash<uint32_t>()(idx.row);
        std::size_t h2 = std::hash<uint32_t>()(idx.column);
        std::size_t h3 = std::hash<void*>()(idx.internalPtr);

        std::size_t seed = h1;
        seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

        return seed;
    };
};

struct ViewLMBRelease : public core::IEventCRTP<ViewLMBRelease>
{
    ViewLMBRelease() {}
    ViewLMBRelease(const ModelIndex& dataIn) : index{dataIn} {}
    const ModelIndex index{};
};

class AbstractModel
{
public:
    virtual ~AbstractModel() = default;
    virtual auto index(const uint32_t row, const uint32_t column,
        const ModelIndex parent) const -> ModelIndex = 0;
    virtual auto data(const ModelIndex idx) const -> std::string = 0;
    virtual auto getRowCount(const ModelIndex parent = ModelIndex{}) const -> uint32_t = 0;
    virtual auto depth(const ModelIndex&) const -> uint32_t { return 0; }
    virtual auto hasChildren(const ModelIndex&) -> bool { return false; }
};
using AbstractModelPtr = std::shared_ptr<AbstractModel>;
} // namespace lav::core