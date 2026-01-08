#pragma once

#include <memory>

#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UIBase.hpp"

namespace src::uielements
{
using namespace elementcomposable;

/**
    @brief Treeview that can be used to display elements in a tree like structure with on
        click opening and closing of parent node UI children.
    
    @note All elements need to have the same Y size as of current limitations.
*/
class UITreeView : public UIPane
{
public:
    struct Item;
    using ItemPtr = std::shared_ptr<Item>;
    using ItemPtrVec = std::vector<ItemPtr>;

    /**
        @brief Base class representing data and generic functionality of a tree item.

        @note Can be extended to provide more data per item as needed.
    */
    struct Item
    {
        Item() = default;
        Item(std::string t, glm::vec4 c) : text{std::move(t)}, color{std::move(c)} {};
        virtual ~Item() = default;

        /**
            @brief Adds a new item to this sub tree.

            @param item The new rvalue item to be added
        */
        auto addItem(ItemPtr&& item) -> void;

        /**
            @brief Adds a new item to this sub tree.

            @param item The new rvalue item to be added
        */
        auto addItem(const ItemPtr& item) -> void;

        /**
            @brief Removes the specified item from this subtree.

            @param item The new value item to be added

            @return True on success. False otherwise.
        */
        auto removeItem(const ItemPtr& item) -> bool;

        std::string text;
        glm::vec4 color;
        int32_t depth{0};
        bool open{true};
        ItemPtrVec subItems;
    };

public:
    UITreeView();
    ~UITreeView() = default;
    UITreeView(const UITreeView&) = delete;
    UITreeView(UIBase&&) = delete;
    auto operator=(const UITreeView&) -> UITreeView& = delete;
    auto operator=(UITreeView&&) -> UITreeView& = delete;

    /**
        @brief Adds a new item to this sub tree.

        @param item The new rvalue item to be added
    */
    auto addItem(ItemPtr&& item) -> void;

    /**
        @brief Adds a new root item to the tree.

        @param item The new item to be added
    */
    auto addItem(const ItemPtr& item) -> void;

    /**
        @brief Removes the specified item from root tree.

        @param item The new rvalue item to be added

        @return True on success. False otherwise.
    */
    auto removeItem(const ItemPtr& item) -> bool;

    /**
        @brief Refreshes the interal tree structure to reflect the new changes.

        @note Needs to be called after a single/bulk modification of the tree
            sturucture (add/remove).
    */
    auto refreshItems() -> void;

    auto setRowSize(const uint32_t value) -> void;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UITreeView);

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(state::UIWindowStatePtr& state) -> void override;
    auto resolveVisibleItems() -> void;

private:
    ItemPtrVec treeRoots_;
    ItemPtrVec flatItems_;
    uint32_t rowSize_{30};
    int32_t topOfTheListIdx_{0};
    int32_t visibleCount_{0};
    int32_t oldTopOfTheListIdx_{-1};
    int32_t oldVisibleCount_{-1};
};
using UITreeViewPtr = std::shared_ptr<UITreeView>;
using UITreeViewWPtr = std::weak_ptr<UITreeView>;
} // namespace src::uielements
