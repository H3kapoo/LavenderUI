#include "SliderRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Node/UISlider.hpp"

namespace lav::core
{
auto SliderRule::getRule() const -> IRule::RuleData
{
    return {getConstructRule(), getAdditionRule()};
}

auto SliderRule::getConstructRule() const -> ConstructRule
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        const auto& ph = ParseHelper::get();
        node::UISliderPtr obj = utils::make<node::UISlider>();
        for (const auto&[key, value] : attribs)
        {
            if (key == "scale")
            {
                obj->getBaseLayoutData().setScale(ph.toScale(value));
            }
            else if (key == "ori" || key == "orientation")
            {
                obj->getBaseLayoutData().setType(ph.toOrientation(value));
            }
            else if (key == "default")
            {
                obj->setScrollValue(ph.toNumber(value));
            }
            else if (key == "from")
            {
                obj->setScrollFrom(ph.toNumber(value));
            }
            else if (key == "to")
            {
                obj->setScrollTo(ph.toNumber(value));
            }
            else if (key == "color")
            {
                obj->setColor(ph.toColor(value));
            }
            else if (key == "kcolor")
            {
                obj->setKnobColor(ph.toColor(value));
            }
            else if (key == "showText" && value == "no")
            {
                // Nothing yet, but we can
            }
        }
        return obj;
    };
}

auto SliderRule::getAdditionRule() const -> AddRule
{
    return [this](node::UIBasePtr parent, node::UIBasePtr child) -> void
    {
        (void)parent;
        (void)child;
        // parent->add(elements);
    };
}
} // namespace lav::core