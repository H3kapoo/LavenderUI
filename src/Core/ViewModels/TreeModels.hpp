#pragma once

#include <vector>
#include <functional>

#include "src/Core/ViewModels/AbstractModel.hpp"
#include "src/Utils/Logger.hpp"

namespace lav::core
{
template<typename T>
struct SimpleTreeItem
{
    T data;
    SimpleTreeItem* parent{nullptr};
    std::vector<SimpleTreeItem*> children;
};

using SimpleTreeItemS = SimpleTreeItem<std::string>;

template<typename T>
class TreeBasicModel : public AbstractModel
{
public:
    TreeBasicModel(SimpleTreeItem<T>* root)
        : root_(root)
    {}

    auto index(const uint32_t row, const uint32_t,
        const ModelIndex parent) const -> ModelIndex override
    {
        if (!parent.isValid()) { return ModelIndex{row, 0, root_}; }

        SimpleTreeItem<T>* data = static_cast<SimpleTreeItem<T>*>(parent.internalPtr);
        if (row >= data->children[parent.row]->children.size())
        { 
            return ModelIndex{};
        }

        ModelIndex mi{row, 0, data->children[parent.row]};
        return mi;
    }

    auto data(const ModelIndex idx) const -> std::string override
    {
        // means we root
        if (!idx.isValid())
        {
            return root_->data;
        }

        SimpleTreeItem<T>* data = static_cast<SimpleTreeItem<T>*>(idx.internalPtr);
        if (idx.row >= data->children.size()) { return "No Data"; }

        return data->children[idx.row]->data;
        // return std::to_string(data_.at(idx.row));
    }

    auto getRowCount(const ModelIndex parent) const -> uint32_t override
    {
        // we root
        if (!parent.isValid()) { return root_->children.size(); }

        SimpleTreeItem<T>* data = static_cast<SimpleTreeItem<T>*>(parent.internalPtr);
        if (parent.row >= data->children.size()) { return 0; }

        return data->children[parent.row]->children.size();
    }

protected:
    SimpleTreeItem<T>* root_{nullptr};
};

// class ListOrderedModel : public AbstractModel
// {
// public:
//     ListOrderedModel(AbstractModel& source)
//         : source_(source)
//         , order_()
//     {
//         order_.reserve(source_.getRowCount());
//         for (uint64_t i = 0; i < source_.getRowCount(); i++)
//         {
//             order_.push_back(source_.index(i, 0, ModelIndex{}));
//         }

//         std::sort(order_.begin(), order_.end(),
//             [this](const auto& miA, const auto& miB) -> bool
//             {
//                 return std::stoi(source_.data(miA)) > std::stoi(source_.data(miB));
//             });
//     }

//     auto index(const uint64_t row, const uint64_t,
//         const ModelIndex) const -> ModelIndex override
//     {
//         if (row >= order_.size()) { return ModelIndex{}; }
//         return ModelIndex{order_[row]};
//     }

//     auto data(const ModelIndex idx) const -> std::string override
//     {
//         if (!idx.isValid()) { return "No Data"; }
//         return source_.data(idx);
//     }

//     auto getRowCount() const -> uint64_t override { return order_.size(); }

// private:
//     const AbstractModel& source_;
//     std::vector<ModelIndex> order_;
// };

// class ListFilteredModel : public AbstractModel
// {
// public:
//     ListFilteredModel(const AbstractModel& source,
//         const std::function<bool(uint64_t)> pred)
//         : mapping_()
//         , source_(source)
//         , pred_(pred)
//     {
//         mapping_.clear();
//         mapping_.reserve(source_.getRowCount());
//         for (uint64_t dIdx = 0; dIdx < source_.getRowCount(); dIdx++)
//         {
//             const std::string data = source_.data(ModelIndex{dIdx});
//             if (pred_(std::stoi(data))) { continue; }
//             mapping_.push_back(source_.index(dIdx, 0, ModelIndex{}));
//         }
//     }

//     auto index(const uint64_t row, const uint64_t,
//         const ModelIndex) const -> ModelIndex override
//     {
//         if (row >= mapping_.size()) { return ModelIndex{}; }
//         return ModelIndex{mapping_[row]};
//     }

//     auto data(const ModelIndex idx) const -> std::string override
//     {
//         if (!idx.isValid()) { return "No Data"; }
//         return source_.data(idx);
//     }

//     auto getRowCount() const -> uint64_t override { return mapping_.size(); }

// private:
//     std::vector<ModelIndex> mapping_;
//     const AbstractModel& source_;
//     const std::function<bool(uint64_t)> pred_;
// };

} // namespace lav::core