#pragma once

#include <LavenderUI/Core/LayoutHandler/BaseCalculator.hpp>
#include <LavenderUI/Utils/Logger.hpp>

namespace lav::core
{
class DropdownCalculator : public BaseCalculator
{
public:
    static auto get() -> DropdownCalculator&;

    /** @brief Calculates the `computedPos` of the child container of the dropdown.

        @param element Element for which to calculate child position
    */
    auto calculatePositionForDropdownElement(node::UIDropdown* element) const -> void;

private:
    DropdownCalculator();
    DropdownCalculator(const DropdownCalculator&) = delete;
    DropdownCalculator(DropdownCalculator&&) = delete;
    auto operator=(const DropdownCalculator&) -> DropdownCalculator& = delete;
    auto operator=(DropdownCalculator&&) -> DropdownCalculator& = delete;

    auto isNestedInsideAnotherDropdown(node::UIDropdown* dropdown) const -> bool;

    utils::Logger log_;
};
} // namespace lav::core