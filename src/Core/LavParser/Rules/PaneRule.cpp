#include "PaneRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Node/UIPane.hpp"

namespace lav::core
{
auto PaneRule::getRule() const -> RuleSignature
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        node::UIPanePtr obj = utils::make<node::UIPane>();
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
        }
        return obj;
    };
}
} // namespace lav::core