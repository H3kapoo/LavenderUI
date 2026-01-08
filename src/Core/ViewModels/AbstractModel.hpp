#pragma once

#include <cstdint>
#include <memory>

#include "src/Core/EventHandler/IEvent.hpp"

namespace lav::core
{
struct ModelIndex
{
    uint32_t row{UINT32_MAX};
    uint32_t column{UINT32_MAX};
    void* internalPtr{nullptr};

    bool isValid() const
    {
        return row != UINT32_MAX && column != UINT32_MAX;
    }
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
};
using AbstractModelPtr = std::shared_ptr<AbstractModel>;
} // namespace lav::core