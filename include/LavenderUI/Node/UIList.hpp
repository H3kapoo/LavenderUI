#pragma once

#include <LavenderUI/Core/ViewModels/AbstractModel.hpp>
#include <LavenderUI/Node/UIPane.hpp>
#include <LavenderUI/Node/UIBase.hpp>

namespace lav::node
{
/* Fwd declare internal use only detail */
class UIViewItem;

/**
    @brief
    Class used to display and cycle around many list items in a performant lazy way.
*/
class UIList : public UIPane
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIList, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UIList);

    auto setModel(core::AbstractModelPtr model) -> void;
    auto setRowSize(const uint32_t value) -> void;
    auto setAlternatingRowEnabled(const bool value = true) -> void;

protected:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(core::UIStatePtr& state) -> void override;

private:
    auto calculateLayout() -> void;
    auto resolveOverflow() -> void;
    auto resolveVisibleItems() -> void;
    auto allocatePool() -> void;
    auto prepareItem(const uint32_t viewRow, std::shared_ptr<UIViewItem>& item) -> void;

private:
    std::vector<std::shared_ptr<UIViewItem>> uiViewItemPool_;
    core::AbstractModelPtr model_;
    uint32_t selectedId_;
    uint32_t tolerance_;
    uint32_t rowSize_;
    int32_t topOfTheListIdx_;
    int32_t oldTopOfTheListIdx_;
    int32_t visibleCount_;
    int32_t oldVisibleCount_;
    bool isAlternatingRowPattern_;
};
using UIListPtr = std::shared_ptr<UIList>;
using UIListWPtr = std::weak_ptr<UIList>;
} // namespace lav::node
