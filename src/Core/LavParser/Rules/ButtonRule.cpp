#include "ButtonRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Node/UIButton.hpp"

namespace lav::core
{
auto ButtonRule::getRule() const -> RuleSignature
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        node::UIButtonPtr obj = utils::make<node::UIButton>();
        for (const auto&[key, value] : attribs)
        {
            if (key == "scale")
            {
                obj->getBaseLayoutData().setScale(parseHelper_.toScale(value));
            }
            else if (key == "text")
            {
                obj->setText(value);
            }
        }
        return obj;
    };
}
} // namespace lav::core