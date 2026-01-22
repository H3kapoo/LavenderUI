#pragma once

#include <vector>
#include <functional>

#include "include/LavenderUI/Core/ViewModels/AbstractModel.hpp"
#include "include/LavenderUI/Utils/Misc.hpp"

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

    auto data(const ModelIndex idx, const EModelRole role) const -> ModelVariant override
    {
        if (!idx.isValid()) { return ModelVariant{}; }

        if (role == EModelRole::DISPLAY)
            return std::to_string(data_.at(idx.row));
        if (role == EModelRole::COLOR)
            return utils::hexToVec4("#e46b6bff");
        if (role == EModelRole::ALTERNATE_COLOR_1)
            return utils::hexToVec4("#adadadff");
        if (role == EModelRole::ALTERNATE_COLOR_2)
            return utils::hexToVec4("#e46b6bff");

        return ModelVariant{};
    }

    auto getRawDisplayData(const ModelIndex idx) const -> uint32_t
    {
        return data_.at(idx.row);
    }

    auto getRowCount(const ModelIndex = ModelIndex{}) const -> uint32_t override { return data_.size(); }

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
                std::string a = std::get<0>(source_.data(miA, EModelRole::DISPLAY));
                std::string b = std::get<0>(source_.data(miB, EModelRole::DISPLAY));
                return std::stoi(a) > std::stoi(b);
            });
    }

    auto index(const uint32_t row, const uint32_t,
        const ModelIndex) const -> ModelIndex override
    {
        if (row >= order_.size()) { return ModelIndex{}; }
        return ModelIndex{order_[row]};
    }

    auto data(const ModelIndex idx, const EModelRole role) const -> ModelVariant override
    {
        if (!idx.isValid()) { return ModelVariant{"No Data"}; }
        return source_.data(idx, role);
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
    {
        rebuild(pred);
    }

    auto rebuild(const std::function<bool(uint32_t)> pred) -> void
    {
        mapping_.clear();
        mapping_.reserve(source_.getRowCount());
        ModelIndex idx{0, 0, nullptr};
        for (uint32_t dIdx = 0; dIdx < source_.getRowCount(); dIdx++)
        {
            idx.row = dIdx;
            // For huge lists, getting the value raw is way better. User needs to create custom Model.
            // Otherwise they need to settle for the generic slow version.
            const std::string data = GET_STR_ROLE2(source_, idx, core::AbstractModel::EModelRole::DISPLAY);
            if (!pred(std::stoi(data))) { continue; }
            mapping_.push_back(source_.index(dIdx, 0, ModelIndex{}));
        }
    }

    auto index(const uint32_t row, const uint32_t,
        const ModelIndex) const -> ModelIndex override
    {
        if (row >= mapping_.size()) { return ModelIndex{}; }
        return ModelIndex{mapping_[row]};
    }

    auto data(const ModelIndex idx, const EModelRole role) const -> ModelVariant override
    {
        if (!idx.isValid()) { return "No Data"; }
        return source_.data(idx, role);
    }

    auto getRowCount(const ModelIndex) const -> uint32_t override { return mapping_.size(); }

private:
    std::vector<ModelIndex> mapping_;
    const AbstractModel& source_;
};

} // namespace lav::core