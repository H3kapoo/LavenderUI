#pragma once

#include "src/Node/UIButton.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIPane.hpp"

namespace lav::node
{
/**
    @brief
    Splitter GUI element used as a container manager holding multiple UIPanes
    that can be resized on mouse drag.
*/

class UISplitPane;
using UISplitPanePtr = std::shared_ptr<UISplitPane>;
using UISplitPaneWPtr = std::weak_ptr<UISplitPane>;

/** @brief Concept for elements that can be added to this SplitPane */
template<typename T>
concept UISplitPaneElement = 
    std::is_base_of_v<UIPane, std::remove_cvref_t<T>> ||
    std::is_base_of_v<UISplitPane, std::remove_cvref_t<T>>;

class UISplitPane : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UISplitPane, "elemVert.glsl", "elemFrag.glsl")

    /**
        @brief Create a new simple pane for this split pane element.

        @note All elements relative sizes of this split pane need to add up to 1.0f

        @param relativeSpace The 0 < x <= 1 relative space the pane shall occupy
        @param minMax Minimum and maximum space the pane shall occupy in pixels

        @return Weak pointer to the newly created pane.
    */
    [[nodiscard]] auto createPane(const float relativeSpace, const glm::ivec2& minMax) -> UIPaneWPtr;

    /**
        @brief Create a new split pane for this split pane element.

        @note All elements relative sizes of this split pane need to add up to 1.0f

        @param relativeSpace The 0 < x <= 1 relative space the pane shall occupy
        @param minMax Minimum and maximum space the pane shall occupy in pixels

        @return Weak pointer to the newly created split pane.
    */
    [[nodiscard]] auto createSubsplit(const float relativeSpace, const glm::ivec2& minMax) -> UISplitPaneWPtr;

    auto getPaneIdx(const uint32_t idx) -> UIPaneWPtr;
    auto getHandleIdx(const uint32_t idx) -> UIButtonWPtr;

private:
    auto onRender(const glm::mat4& projection) -> void override;
    auto onLayout() -> void override;
    auto onEvent(node::UIStatePtr& state) -> void override;

    template<UISplitPaneElement T> // constrain to Pane,SplitPane
    auto create(const float relativeSpace, const glm::ivec2& minMax) -> std::weak_ptr<T>;

private:
    glm::ivec2 mousePos_{-1, -1};
    std::optional<lav::Cursor> wantedCursor_{std::nullopt};
    uint32_t draggedHandleId_{0};
};
} // namespace lav::node
