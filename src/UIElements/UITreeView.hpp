#pragma once

#include <memory>

#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIPane.hpp"

namespace src::uielements
{
using namespace elementcomposable;

/**
    @brief:
        Treeview that can be used to display elements in a tree like structure with on
        click opening and closing of parent node UI children.
    
    @notes:
    (1) All elements need to have the same Y size as of current limitations.
*/
class UITreeView : public UIPane
{
public:
    struct Item;
    using ItemPtr = std::shared_ptr<Item>;
    using ItemPtrVec = std::vector<ItemPtr>;

    struct Item
    {
        Item() = default;
        Item(std::string t, glm::vec4 c) : text{std::move(t)}, color{std::move(c)} {};
        virtual ~Item() = default;
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
    UITreeView& operator=(const UITreeView&) = delete;
    UITreeView& operator=(UITreeView&&) = delete;

    auto addItem(const ItemPtr& item) -> void;
    auto refreshItems() -> void;

    auto setRowSize(const uint32_t value) -> void;

    INSERT_TYPEINFO(UITreeView);

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(state::UIWindowStatePtr& state) -> void override;


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
