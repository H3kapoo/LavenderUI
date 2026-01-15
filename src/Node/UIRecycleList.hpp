#pragma once

#include "src/Core/ViewModels/AbstractModel.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Node/UIPane.hpp"
#include "src/Node/UIBase.hpp"

namespace lav::node
{
/**
    @brief
    Class used to display and cycle around many list items in a performant lazy way.
*/
class UIRecycleList : public UIPane
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIRecycleList, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UIRecycleList);

    auto setModel(core::AbstractModelPtr model) -> void;
    auto setRowSize(const uint32_t value) -> void;

protected:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(UIStatePtr& state) -> void override;

private:
    auto calculateLayout() -> glm::i64vec2;
    auto resolveVisibleItems() -> void;
    auto growVisibleItems(const int32_t count, const bool atFront) -> void;
    auto shrinkVisibleItems(const int32_t count, const bool atFront) -> void;
    auto fillVisibleItems(const uint32_t viewRowIdx, const int32_t count, const bool atFront) -> void;
    auto getElementAt(const uint32_t index) -> UIBasePtr;

private:
    core::AbstractModelPtr model_;
    uint64_t selectedId_;
    int64_t topOfTheListIdx_;
    int64_t oldTopOfTheListIdx_;
    int32_t visibleCount_;
    int32_t oldVisibleCount_;
    uint32_t tolerance_;
    uint32_t rowSize_;

    uint32_t currentVisibleRowStartIdx_{0};
    uint32_t currentVisibleRowEndIdx_{0};
    uint32_t wantedVisibleRowStartIdx_{0};
    uint32_t wantedVisibleRowEndIdx_{0};

    std::vector<UIButtonPtr> uiButtonPool_;
};
using UIRecycleListPtr = std::shared_ptr<UIRecycleList>;
using UIRecycleListWPtr = std::weak_ptr<UIRecycleList>;
} // namespace lav::node
