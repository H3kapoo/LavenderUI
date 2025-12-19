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
    std::is_same_v<UIPane, std::remove_cvref_t<T>> ||
    std::is_same_v<UISplitPane, std::remove_cvref_t<T>>;

class UISplitPane : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UISplitPane, "elemVert.glsl", "elemFrag.glsl")

    auto createPane(UIPanePtr&& pane, const float relativeSpace, const glm::ivec2& minMax) -> void;
    auto createSubsplit(UISplitPanePtr&& subSplit, const float relativeSpace,
        const glm::ivec2& minMax) -> void;
    [[nodiscard]] auto createPane(const float relativeSpace, const glm::ivec2& minMax) -> UIPaneWPtr;
    [[nodiscard]] auto createSubsplit(const float relativeSpace, const glm::ivec2& minMax) -> UISplitPaneWPtr;

    auto getPaneIdx(const uint32_t idx) -> UIPaneWPtr;
    auto getHandleIdx(const uint32_t idx) -> UIButtonWPtr;

private:
    auto onRender(const glm::mat4& projection) -> void override;
    auto onLayout() -> void override;
    auto onEvent(node::UIStatePtr& state) -> void override;

    template<UISplitPaneElement T> // constrain to Pane,SplitPane
    // template<typename T> // constrain to Pane,SplitPane
    auto create(std::shared_ptr<T>&& uiElement, const float relativeSpace,
        const glm::ivec2& minMax) -> std::weak_ptr<T>;

private:
    glm::ivec2 mousePos_;
    std::optional<lav::Cursor> wantedCursor_;
    uint32_t draggedHandleId_;
};
} // namespace lav::node
