#pragma once

#include <vector>
#include <functional>

#include <LavenderUI/Core/ViewModels/AbstractModel.hpp>
#include <LavenderUI/Utils/Logger.hpp>

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
        const ModelIndex parentIdx) const -> ModelIndex override
    {
        SimpleTreeItem<T>* parentItem = parentIdx.isValid()
            ? static_cast<SimpleTreeItem<T>*>(parentIdx.internalPtr)
            : root_;

        return ModelIndex{row, 0, parentItem->children[row]};
    }

    auto parent(const ModelIndex idx) const -> ModelIndex
    {
        SimpleTreeItem<T>* item = idx.isValid()
            ? static_cast<SimpleTreeItem<T>*>(idx.internalPtr)
            : root_;
        
        if (item == root_) { return ModelIndex{}; }

        SimpleTreeItem<T>* parentItem = item->parent;
        if (!parentItem) { return ModelIndex{}; }

        SimpleTreeItem<T>* grandParentItem = parentItem->parent;
        if (!grandParentItem) { return ModelIndex{}; }

        for (uint32_t i = 0; i < grandParentItem->children.size(); ++i)
        {
            if (grandParentItem->children[i] == parentItem)
            {
                return ModelIndex{i, 0, parentItem};
            }
        }

        return ModelIndex{};
    }
    
    auto hasChildren(const ModelIndex& idx) -> bool override
    {
        SimpleTreeItem<T>* item = idx.isValid()
            ? static_cast<SimpleTreeItem<T>*>(idx.internalPtr)
            : root_;
        return !item->children.empty();
    }

    auto depth(const ModelIndex& idx) const -> uint32_t override
    {
        SimpleTreeItem<T>* item = idx.isValid()
            ? static_cast<SimpleTreeItem<T>*>(idx.internalPtr)
            : root_;
        
        uint32_t depth{0};
        item = item->parent;
        while (item != root_)
        {
            item = item->parent;
            depth++;
        }

        return depth;
    }

    auto data(const ModelIndex idx, const EModelRole role) const -> ModelVariant override
    {
        SimpleTreeItem<T>* item = idx.isValid()
            ? static_cast<SimpleTreeItem<T>*>(idx.internalPtr)
            : root_;

        if (!idx.isValid()) { return ModelVariant{}; }

        if (role == EModelRole::DISPLAY)
            return item->data;
        if (role == EModelRole::COLOR)
            return utils::hexToVec4("#e46b6bff");
        if (role == EModelRole::ALTERNATE_COLOR_1)
            return utils::hexToVec4("#adadadff");
        if (role == EModelRole::ALTERNATE_COLOR_2)
            return utils::hexToVec4("#e46b6bff");

        return ModelVariant{};
    }

    auto getRowCount(const ModelIndex idx) const -> uint32_t override
    {
        SimpleTreeItem<T>* item = idx.isValid()
            ? static_cast<SimpleTreeItem<T>*>(idx.internalPtr)
            : root_;
        return item->children.size();
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