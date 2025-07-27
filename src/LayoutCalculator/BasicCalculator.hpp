#pragma once

#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIPane.hpp"

namespace src::layoutcalculator
{
/**
    @brief:
        Simple layout calculator for scaling and placing GUI elements in their appropriate place.
        Can be used as a template to derive other layout mechanics.
    
    @notes:
    (1) Singleton class as each layout pass shall be stateless for a better view of the layout pipeline.
    (2) There is a limitation on using shared_from_this() in the derived classes that inherit UIBase.
        Since UIBase inherits enable_shared_from_this<UIBase>, the shared_from_this() returned when used inside
        a derived class is of type UIBase and it will corrupt the shared/weak ptr control block when trying to do
        sharedParent->getSomethingShared/Weak inside the layout calculator (derived_func(shared_from_this()).
        The workaround for this is to use raw pointers and call the functions with "this" instead of
        "shared_from_this". It is safe anyway as the class doesnt save anything or delete the pointers.

        Ex of explained:
        in calculator:
        castedParent->getSomeInternalUIElementShared() => this will crash
                                                          if it was called from derived class with
                                                          parent = shared_from_this
*/
class BasicCalculator
{
public:
    static auto get() -> BasicCalculator&;

    auto calculate(const uielements::UIBasePtr& parent) -> void;
    auto calcPaneElements(uielements::UIPane* parent, const glm::vec2 scrollData) const -> void;
    auto calcPaneSliders(uielements::UIPane* parent) const -> glm::vec2;
    auto calcPaneElementsAddScrollToPos(uielements::UIPane* parent, const glm::vec2 scrollData) const -> void;

    auto calcElementsPos(uielements::UIBase* parent, const glm::vec2 scrollData = {}) const -> void;
    auto calcElementsScale(uielements::UIBase* parent, const glm::vec2 scrollData = {}) const -> void;

    auto calcOverflow(uielements::UIBase* parent, const glm::vec2 scrollData) const -> glm::vec2;
};
} // namespace src::layoutcalculator
