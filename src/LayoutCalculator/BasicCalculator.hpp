#pragma once

#include "src/ElementComposable/LayoutAttribs.hpp"
#include "src/UIElements/UIBase.hpp"

namespace src::layoutcalculator
{
class BasicCalculator
{
public:
    static auto get() -> BasicCalculator&;

    auto calculate(const uielements::UIBasePtr& parent) -> void;
};
} // namespace src::layoutcalculator
