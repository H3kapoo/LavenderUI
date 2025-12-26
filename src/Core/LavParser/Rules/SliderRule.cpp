#include "SliderRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Node/UISlider.hpp"

namespace lav::core
{
auto SliderRule::construct(const RuleMap&, const XmlNode& xmlNode,
    node::UIBasePtr, const bool) const -> node::UIBasePtr
{
    node::UISliderPtr slider = utils::make<node::UISlider>();
    parseAndApply(slider, xmlNode->attributes);
    return slider;
}

auto SliderRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UISliderPtr slider = utils::as<node::UISlider>(object);
    for (const auto&[key, value] : attribs)
    {
        if (key == "scale")
        {
            slider->getBaseLayoutData().setScale(ph.toScale(value));
        }
        else if (key == "ori" || key == "orientation")
        {
            slider->getBaseLayoutData().setType(ph.toOrientation(value));
        }
        else if (key == "default")
        {
            slider->setScrollValue(ph.toNumber(value));
        }
        else if (key == "from")
        {
            slider->setScrollFrom(ph.toNumber(value));
        }
        else if (key == "to")
        {
            slider->setScrollTo(ph.toNumber(value));
        }
        else if (key == "color")
        {
            slider->setColor(ph.toColor(value));
        }
        else if (key == "kcolor")
        {
            slider->setKnobColor(ph.toColor(value));
        }
        else if (key == "showText" && value == "no")
        {
            // Nothing yet, but we can
        }
    }
}
} // namespace lav::core