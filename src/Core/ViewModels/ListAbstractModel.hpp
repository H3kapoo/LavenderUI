#pragma once

#include "src/Utils/Logger.hpp"
#include <cstdint>
#include <memory>
#include <vector>
#include <functional>
#include "src/Core/EventHandler/IEvent.hpp"

namespace lav::core
{

struct ModelIndex
{
    uint64_t row{UINT64_MAX};

    bool isValid() const
    {
        return row != UINTMAX_MAX;
    }
};

struct ViewLMBRelease : public core::IEventCRTP<ViewLMBRelease>
{
    ViewLMBRelease() {}
    ViewLMBRelease(const ModelIndex& dataIn) : index{dataIn} {}
    const ModelIndex index{};
};

class ListAbstractModel
{
public:
    virtual ~ListAbstractModel() = default;
    virtual auto index(const uint64_t row, const uint64_t column,
        const ModelIndex parent) const -> ModelIndex = 0;
    virtual auto data(const ModelIndex idx) const -> std::string = 0;
    virtual auto getRowCount() const -> uint64_t = 0;
};

using ListAbstractModelPtr = std::shared_ptr<ListAbstractModel>;

class ListBasicModel : public ListAbstractModel
{
public:
    ListBasicModel(const std::vector<uint64_t>& data)
        : data_(data)
    {}

    auto index(const uint64_t row, const uint64_t,
        const ModelIndex) const -> ModelIndex override
    {
        if (row >= data_.size()) { return ModelIndex{}; }
        return ModelIndex(row);
    }

    auto data(const ModelIndex idx) const -> std::string override
    {
        return std::to_string(data_.at(idx.row));
    }

    auto getRowCount() const -> uint64_t override { return data_.size(); }

protected:
    const std::vector<uint64_t>& data_;
};

class ListOrderedModel : public ListAbstractModel
{
public:
    ListOrderedModel(ListAbstractModel& source)
        : source_(source)
        , order_()
    {
        order_.reserve(source_.getRowCount());
        for (uint64_t i = 0; i < source_.getRowCount(); i++)
        {
            order_.push_back(source_.index(i, 0, ModelIndex{}));
        }

        std::sort(order_.begin(), order_.end(),
            [this](const auto& miA, const auto& miB) -> bool
            {
                return std::stoi(source_.data(miA)) > std::stoi(source_.data(miB));
            });
    }

    auto index(const uint64_t row, const uint64_t,
        const ModelIndex) const -> ModelIndex override
    {
        if (row >= order_.size()) { return ModelIndex{}; }
        return ModelIndex{order_[row]};
    }

    auto data(const ModelIndex idx) const -> std::string override
    {
        if (!idx.isValid()) { return "No Data"; }
        return source_.data(idx);
    }

    auto getRowCount() const -> uint64_t override { return order_.size(); }

private:
    const ListAbstractModel& source_;
    std::vector<ModelIndex> order_;
};

class ListFilteredModel : public ListAbstractModel
{
public:
    ListFilteredModel(const ListAbstractModel& source,
        const std::function<bool(uint64_t)> pred)
        : mapping_()
        , source_(source)
        , pred_(pred)
    {
        mapping_.clear();
        mapping_.reserve(source_.getRowCount());
        for (uint64_t dIdx = 0; dIdx < source_.getRowCount(); dIdx++)
        {
            const std::string data = source_.data(ModelIndex{dIdx});
            if (pred_(std::stoi(data))) { continue; }
            mapping_.push_back(source_.index(dIdx, 0, ModelIndex{}));
        }
    }

    auto index(const uint64_t row, const uint64_t,
        const ModelIndex) const -> ModelIndex override
    {
        if (row >= mapping_.size()) { return ModelIndex{}; }
        return ModelIndex{mapping_[row]};
    }

    auto data(const ModelIndex idx) const -> std::string override
    {
        if (!idx.isValid()) { return "No Data"; }
        return source_.data(idx);
    }

    auto getRowCount() const -> uint64_t override { return mapping_.size(); }

private:
    std::vector<ModelIndex> mapping_;
    const ListAbstractModel& source_;
    const std::function<bool(uint64_t)> pred_;
};

} // namespace lav::core