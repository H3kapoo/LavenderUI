#pragma once

#include <vector>
#include <functional>

#include "src/Core/ViewModels/AbstractModel.hpp"

namespace lav::core
{
class ListBasicModel : public AbstractModel
{
public:
    ListBasicModel(const std::vector<uint32_t>& data)
        : data_(data)
    {}

    auto index(const uint32_t row, const uint32_t,
        const ModelIndex) const -> ModelIndex override
    {
        if (row >= data_.size()) { return ModelIndex{}; }
        return ModelIndex(row);
    }

    auto data(const ModelIndex idx) const -> std::string override
    {
        return std::to_string(data_.at(idx.row));
    }

    auto getRowCount(const ModelIndex) const -> uint32_t override { return data_.size(); }

protected:
    const std::vector<uint32_t>& data_;
};

class ListOrderedModel : public AbstractModel
{
public:
    ListOrderedModel(AbstractModel& source)
        : source_(source)
        , order_()
    {
        order_.reserve(source_.getRowCount());
        for (uint32_t i = 0; i < source_.getRowCount(); i++)
        {
            order_.push_back(source_.index(i, 0, ModelIndex{}));
        }

        std::sort(order_.begin(), order_.end(),
            [this](const auto& miA, const auto& miB) -> bool
            {
                return std::stoi(source_.data(miA)) > std::stoi(source_.data(miB));
            });
    }

    auto index(const uint32_t row, const uint32_t,
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

    auto getRowCount(const ModelIndex) const -> uint32_t override { return order_.size(); }

private:
    const AbstractModel& source_;
    std::vector<ModelIndex> order_;
};

class ListFilteredModel : public AbstractModel
{
public:
    ListFilteredModel(const AbstractModel& source,
        const std::function<bool(uint32_t)> pred)
        : mapping_()
        , source_(source)
        , pred_(pred)
    {
        mapping_.clear();
        mapping_.reserve(source_.getRowCount());
        for (uint32_t dIdx = 0; dIdx < source_.getRowCount(); dIdx++)
        {
            const std::string data = source_.data(ModelIndex{dIdx, 0, nullptr});
            if (pred_(std::stoi(data))) { continue; }
            mapping_.push_back(source_.index(dIdx, 0, ModelIndex{}));
        }
    }

    auto index(const uint32_t row, const uint32_t,
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

    auto getRowCount(const ModelIndex) const -> uint32_t override { return mapping_.size(); }

private:
    std::vector<ModelIndex> mapping_;
    const AbstractModel& source_;
    const std::function<bool(uint32_t)> pred_;
};

} // namespace lav::core