#pragma once

#include "src/ElementComposable/LayoutBase.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIPane.hpp"

namespace src::uielements
{
using namespace elementcomposable;

/**
    @brief Splitter GUI element used as a container manager holding multiple UIPanes
        that can be resize on mouse drag.
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

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UISplitPane);

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(state::UIWindowStatePtr& state) -> void override;

private:
    glm::ivec2 prevMousePos{-1, -1};
};
using UISplitPanePtr = std::shared_ptr<UISplitPane>;
using UISplitPaneWPtr = std::weak_ptr<UISplitPane>;
} // namespace src::uielements
