#pragma once

#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIPane.hpp"

namespace src::uielements
{
using namespace elementcomposable;

/**
    @brief Splitter GUI element used as a container manager holding multiple UIPanes
        that can be resized on mouse drag.
*/
class UISplitPane : public UIBase
{
public:
    UISplitPane();
    virtual ~UISplitPane() = default;
    UISplitPane(const UISplitPane&) = delete;
    UISplitPane(UIBase&&) = delete;
    auto operator=(const UISplitPane&) -> UISplitPane& = delete;
    auto operator=(UISplitPane&&) -> UISplitPane& = delete;

    auto createPanes(const std::vector<float> startFractions) -> void;

    auto getPaneIdx(const uint32_t idx) -> UIPaneWPtr;
    auto getHandleIdx(const uint32_t idx) -> UIButtonWPtr;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UISplitPane);

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(state::UIWindowStatePtr& state) -> void override;

    auto adjustPane(const glm::vec2 handlesOccupiedSpace, const uint32_t handleIdx,
        const bool fromResize = false) -> void;

private:
    glm::ivec2 mousePos_{-1, -1};
    glm::ivec2 wsDelta_{-1, -1};
    uint32_t draggedHandleId_{0};
};
using UISplitPanePtr = std::shared_ptr<UISplitPane>;
using UISplitPaneWPtr = std::weak_ptr<UISplitPane>;
} // namespace src::uielements
