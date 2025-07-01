#include "BasicCalculator.hpp"

namespace src::layoutcalculator
{
using namespace elementcomposable;

auto BasicCalculator::get() -> BasicCalculator&
{
    static BasicCalculator instance;
    return instance;
}

auto BasicCalculator::calculate(const uielements::UIBasePtr& parent) -> void
{
    const auto& pLayout = parent->getLayout();
    auto& elements = parent->getElements();

    for (auto& element : elements)
    {
        auto& eLayout = element->getLayout();
        if (eLayout.scale.x.type == LayoutAttribs::ScaleType::PX)
        {
            eLayout.cScale.x = eLayout.scale.x.val;
        }
        else if (eLayout.scale.x.type == LayoutAttribs::ScaleType::REL)
        {
            eLayout.cScale.x = pLayout.cScale.x * eLayout.scale.x.val;
        }

        if (eLayout.scale.y.type == LayoutAttribs::ScaleType::PX)
        {
            eLayout.cScale.y = eLayout.scale.y.val;
        }
        else if (eLayout.scale.y.type == LayoutAttribs::ScaleType::REL)
        {
            eLayout.cScale.y = pLayout.cScale.y * eLayout.scale.y.val;
        }
    }

    if (pLayout.type == LayoutAttribs::Type::HORIZONTAL)
    {
        glm::vec2 start{pLayout.cPos + pLayout.tempPosOffset};
        for (auto& element : elements)
        {
            auto& eLayout = element->getLayout();
            eLayout.cPos = start;
            // start.x += eLayout.scale.x.val;
            start.x += eLayout.cScale.x;
        }
    }
}
} // namespace src::layoutcalculator
