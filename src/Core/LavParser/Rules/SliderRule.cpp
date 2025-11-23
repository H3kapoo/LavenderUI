#include "SliderRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Node/UISlider.hpp"

namespace lav::core
{
auto SliderRule::getRule() const -> RuleSignature
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        node::UISliderPtr obj = utils::make<node::UISlider>();
        for (const auto&[key, value] : attribs)
        {
            if (key == "scale")
            {
                obj->getBaseLayoutData().setScale(parseHelper_.toScale(value));
            }
            else if (key == "ori" || key == "orientation")
            {
                obj->getBaseLayoutData().setType(parseHelper_.toOrientation(value));
            }
            else if (key == "default")
            {
                obj->setScrollValue(parseHelper_.toNumber(value));
            }
            else if (key == "from")
            {
                obj->setScrollFrom(parseHelper_.toNumber(value));
            }
            else if (key == "to")
            {
                obj->setScrollTo(parseHelper_.toNumber(value));
            }
        }
        return obj;
    };
}
} // namespace lav::core